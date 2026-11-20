# AGENTS.md

C++20 voxel engine (OpenGL 3.3 / GLFW / glad / ImGui / oneTBB). ~1,700 lines across 26
files, all in `src/`.

There are **no tests, no linter config, and no `.clang-format`** in this repo. Don't look for
a `test`/`lint` command — there isn't one. CI exists (`.github/workflows/build.yml`), but it
only builds Linux/Clang and Windows/MSVC; it does not run anything. The quality gate is the
compiler: warnings are errors, so `cmake --build` failing on a diagnostic is normal and
expected. A full review with prioritized fixes lives in `CODE_REVIEW.md` (and
`CODE_REVIEW.pdf`).

## Warning flags are real, and the tree is clean

`CMakeLists.txt:98-112` applies `-Wall -Wextra -Werror -pedantic-errors` on
`CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"` (and `/W4 /WX /permissive-` on MSVC). The
condition is genuinely evaluated — it is not the quoted-string bug it used to be.

All 13 translation units in `src/` compile clean under those flags in CI with `clang++`
(Linux) and MSVC (Windows). If you touch a file, keep it that way; `-Werror` means any new
warning breaks the build.

For a fast single-file check without a full CMake configure:

```bash
clang++ -std=c++20 -fsyntax-only -Wall -Wextra -Werror -pedantic-errors \
    -Iinclude/glad -Iinclude/imgui -Iinclude/fastnoise src/<file>.cpp
```

No TBB include path is needed on a machine where oneTBB is installed system-wide
(`/usr/include/oneapi`). Only add `-I<builddir>/_deps/tbb-src/include` if it isn't.

Two idioms are used deliberately to stay quiet under `-Werror`: `[[nodiscard]]` on
functions whose result must not be dropped (`ChunkManager::GetChunk`,
`ChunkToWorldCoords`/`WorldToChunkCoords`, the `CreateMesh` overrides, …) and
`[[maybe_unused]]` on genuinely dead members.

## C++ standard

CMake sets C++20. The only C++20-only construct in `src/` is
`std::unordered_set::contains` at `chunkmanager.cpp:138`; replacing it with
`find() != end()` makes the tree C++17-clean. `CODE_REVIEW.md` argues for staying on C++20
because `std::jthread` and `std::span` are the right tools for the worker/queue redesign.

## Build and run

`build/` is gitignored. If a configured tree is present (Ninja + `clang++` + Debug):

```bash
cmake --build build/                # incremental
./build/desertedcraft               # currently must be run FROM build/
```

If it is missing, configure it. README documents the variants (`build/`,
`build-gcc/Debug/`, `build-clang/Debug/`, `build-msvc/`). CMake uses system glfw/glm/oneTBB
when `find_package` succeeds; otherwise it **clones and builds them from source**, which is
slow. CMake deprecation warnings from TBB's own CMakeLists are not your problem:

```bash
cmake -B build/ -G Ninja -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
```

Gotchas:
- **Currently must be run from the directory containing `assets/`.** Shaders are loaded via
  relative paths (`renderer.cpp:9-10` hardcodes `./assets/shaders/...`). A `POST_BUILD` step
  (`CMakeLists.txt:114-118`) symlinks `assets/` next to the binary, which is why running from
  `build/` works. `CODE_REVIEW.md` recommends replacing this with an `ASSET_DIR` compile
  definition so the binary works from anywhere.
- Needs a real display/GL context. It cannot be verified headless — build success is the
  only automated signal.
- There is no root `compile_commands.json` (the old symlink is gone). The live one is
  `build/compile_commands.json` once the tree is configured; point tools at that.

## Threading and the OpenGL-context invariant

`Application::Run()` spawns `Constants::NUM_WORKERS` (9, of `NUM_THREADS` = 10) threads
running `ChunkManager::Dispatch`, which `pop` coords from `m_workQueue`, call
`GenerateChunk`, and `emplace` the result. `ChunkManager::Update()` runs on the **main
thread only** and does render-list rebuild, job dispatch, GPU upload, and unloading.

`m_chunkPtrMap` is a plain `std::unordered_map` shared between those threads. **As of commit
`41b708b` the lock scopes themselves are correct** — the four unsynchronized accesses
previously listed here (`GetChunk` unlocked, `Update`'s `find` before the lock, the
GPU-upload loop's post-release dereference, and `Unload`'s split `find`/`erase` scopes) have
all been fixed. There is no lock recursion either: the four `scoped_lock` scopes in `Update()`
are sequential rather than nested, and the `Unload()` call at the end is outside all of them.

Remaining problems, ranked. This list was re-verified against the tree; the full review
with fixes is in `CODE_REVIEW.md`.

1. **`emplace` on a duplicate key destroys a `Chunk` on a worker thread** —
   `chunkmanager.cpp:224`. `unordered_map::emplace` on an existing key is a no-op and
   destroys the passed `unique_ptr` at the end of the full expression, on the worker. That
   runs `~Chunk` → `~Mesh` (`mesh.cpp:41-47`) → `glBindVertexArray` / `glBindBuffer` /
   `glDeleteBuffers` / `glDeleteVertexArrays` with no current context. Currently masked by
   bug 2 (duplicate coords are never queued), so it is latent — but **fixing 2 without fixing
   this converts a missing-chunk bug into a GL crash.** Check the `emplace` return value, or
   hand losers to a main-thread trash queue drained in `Update()`.
2. **`m_jobsQueuedList` is never cleared on unload, so chunks are permanently lost** —
   `chunkmanager.cpp:66-78`. `Unload()` erases from `m_chunkPtrMap` only. The sole
   `m_jobsQueuedList` erase is at `chunkmanager.cpp:145`, in the `else` branch, which only
   runs for coords in the *current* render list and only when the block at line 123 executes.
   A chunk is unloaded at `max(|dx|,|dz|) >= CHUNK_DISTANCE_HORIZONTAL/2 + 3`
   (`chunkmanager.cpp:188`), i.e. *after* it has already left the render list (relative range
   `[-CDH/2, CDH/2-1]`). So the coord stays in `m_jobsQueuedList` forever and the `contains`
   check at line 138 permanently suppresses re-generation. Repro: stand still until the
   initial 64 chunks finish, walk more than 7 chunks away, walk back — permanent holes.
   Compounding it, the whole dispatch block is gated on
   `m_currPlayerChunkCoords != newCoords || m_chunkList.empty()`, so while standing still
   nothing is ever re-queued.
3. **Raw pointers escape the critical section** — `GetChunk` (`chunkmanager.cpp:56-64`)
   returns `iterator->second.get()` and it is dereferenced at `application.cpp:117` with no
   lock. Same shape in the GPU-upload loop (`chunkmanager.cpp:154-164`): the `Mesh*` is
   extracted under the lock, `BufferData()` is called outside it. Both are safe *only*
   because the sole `erase` is in `Unload`, reached from `Update` on the same thread. The
   mutex gives false confidence — the return value is the unprotected part. Draining a worker
   trash queue (fix 1) or unloading in the same frame after the draw loop would turn this
   into a use-after-free.
4. **No join guard** — `application.cpp:87-179`. `workers` are joined at lines 177-179, which
   is only reached by falling out of the while loop. Any throw from lines 98-173 skips both
   `m_workQueue.abort()` and the joins, and `~vector<thread>` on joinable threads calls
   `std::terminate`. Needs a scope guard or `std::jthread`.
5. **GL teardown happens after `glfwTerminate()`** — `application.cpp:74`. The destructor body
   runs before members are destroyed, so `~GameState` → `~ChunkManager` → `~Chunk` → `~Mesh`
   issues `glDeleteBuffers`/`glDeleteVertexArrays` on a terminated context. ImGui is never
   shut down, `Shader` never calls `glDeleteProgram`, and `Window` never calls
   `glfwDestroyWindow`. Also, `application.h:20-22` declares `m_rendererPtr` before
   `m_gameStatePtr`, so `Renderer` (which holds `const Camera&`) is destroyed *after*
   `GameState` — a latent dangling reference.
6. **The shared `Mesher` is called concurrently through non-const members** —
   `chunkmanager.cpp:114`. All 9 workers call `m_mesherPtr->CreateMesh(...)` on the single
   shared instance, and both `Mesher::CreateMesh` (`mesher.h:11`) and the protected
   `Mesher::BuildFace` (`mesher.h:18`) are non-const member functions. They touch no members
   today — `BuildFace` only reads the `const` globals in `cube.h:23,32,88` and appends to a
   caller-local vector — so it is benign, but nothing in the type system or a comment records
   the requirement. Marking both `const` makes it checkable.
7. **Shader compile/link failures only log and continue** — `shader.cpp:73-99`. The
   constructor proceeds with a bad program, and `glDeleteProgram` is never called even on
   success. Throw `std::runtime_error` instead.
8. **`Block` is 4 bytes, so a 100^3 chunk costs 4 MB** — `block.h:27`. 64 render chunks =
   256 MB; the unload margin lets the map hold ~169 chunks (~676 MB). A
   `BlockType : std::uint8_t` stored directly in `std::array` cuts this 4x and improves
   mesher cache behavior.
9. **`Mesh` copies the vertex vector and never frees it after upload** — `mesh.cpp:6-7`
   copies the caller's vector; `mesh.cpp:55-74` uploads but keeps `m_vertices`, retaining
   ~200-300 MB across 64 chunks. Take the vector by value and move, store a `GLsizei`
   vertex count, and clear the vector after `BufferData()`.
10. **Window resize doesn't update the projection aspect** — `window.cpp:69-75` updates only
    `glViewport`; `renderer.cpp:22-24` hardcodes `SCR_WIDTH/SCR_HEIGHT`. Resizing stretches
    the image.

Invariants to preserve when touching this code:

- **OpenGL calls must stay on the main thread.** Worker threads have no GL context, but
  `~Mesh` calls `glDeleteBuffers`/`glDeleteVertexArrays` unconditionally, so destroying a
  `Chunk` off the main thread corrupts GL state. `Mesh::BufferData()` is main-thread-only by
  design. Note `~Mesh` also binds name 0, so even a zero-initialized handle changes GL state;
  `glDelete*` on name 0 is a no-op per the spec.
- **Publication is correctly ordered.** A worker fully constructs the `Chunk`, then
  `emplace` takes `m_mutex` at `chunkmanager.cpp:223`; the main thread only ever reads
  published chunks under that same mutex. While the map is shared, do not "optimize" the
  lock away on either side. `CODE_REVIEW.md` recommends making the map main-thread-only via
  a completed-chunk queue, which removes the lock entirely.
- **Shutdown** works only via `chunkManager.m_workQueue.abort()` throwing `user_abort`,
  which `Dispatch` catches to break (`application.cpp:175`, `chunkmanager.cpp:213-216`).
  There is no `running` flag any more (removed in `94c26a2`); `pop()` blocks, so a flag would
  not be a shutdown mechanism regardless. In-flight chunks still `emplace` after `abort()`
  and that is harmless.
- `m_workQueue` is a `concurrent_bounded_queue` constructed **without a capacity**, so it is
  effectively unbounded and `push` at `chunkmanager.cpp:139` never blocks. Giving it a real
  capacity would stall the main thread — that is why it sits outside the `scoped_lock` scope.
- `m_workQueue` is a public member and `Application` aborts it directly.
- `m_chunkList`, `m_chunkUnloadList`, `m_jobsQueuedList`, and `m_currPlayerChunkCoords` are
  main-thread-only. They need no lock — and must not get one, since `Unload()` re-locking
  while `Update()` holds the mutex would self-deadlock on the non-recursive `m_mutex`.

Verified-safe; do not "fix" these:

- **`m_noise` is shared by all 9 workers and that is fine.** In this vendored FastNoiseLite,
  `GetNoise` / `TransformNoiseCoordinate` / `GenNoiseSingle` / `SinglePerlin` are all `const`,
  there is no mutable permutation table (coordinates are hashed from the seed per call), and
  the only members are `mSeed` plus scalar config. The only mutation is
  `SetNoiseType` at `chunkmanager.cpp:53`, in the constructor, before any worker starts.
  Calling `SetSeed`/`SetNoiseType` after the workers spawn would be a real race.
- `std::hash<glm::ivec3>` (`chunkmanager.h:22-30`) omits `y` from the hash, so every chunk in
  a vertical column collides — but `std::equal_to<glm::ivec3>` compares all three components,
  so this is a perf nit, not a correctness bug. Moot while `CHUNK_DISTANCE_VERTICAL` is 1.
- **The old `m_camera`/`chunkManager` init-order trap is fixed.** Commit `f59605d` moved
  `Camera m_camera;` ahead of `ChunkManager chunkManager;` in `gamestate.h` (now lines 24 and
  27), and the mem-init list at `gamestate.cpp:7` matches. Don't reintroduce the old order.
- **`renderList` in `application.cpp:78` is safe.** It binds a
  `const std::vector<glm::ivec3>&` to the vector *object*, not to its elements. `clear()` +
  refill (`chunkmanager.cpp:126-127`) does not invalidate that reference; only
  iterators/pointers into the old storage would be invalidated. Keep indexing with
  `renderList[i]`, not with a cached iterator.
- **`doneVec` is not an out-of-bounds write today.** `application.cpp:95` sizes it with
  `FINAL_CHUNK_DISTANCE`, and `BuildRenderList` produces exactly `H*V*H` entries, which is the
  same product. The coupling is fragile, not UB; prefer `m_chunkList.size()` or a counter.


## Adding a source file

`CMakeLists.txt` lists sources explicitly in `add_executable(...)` — no globbing. New `.cpp`
files must be added there by hand or they will not be compiled.

## Layout / dependencies

- `include/` is **vendored third-party code**: `glad/`, `imgui/`, `fastnoise/`. Do not
  reformat, lint, or "fix" anything there.
- TBB is *not* in `include/`; CMake uses the system install when `find_package(TBB)`
  succeeds, otherwise it is `FetchContent`-ed into `build/_deps/tbb-src/`.
- There is no single `include/` root. `target_include_directories` lists `include/glad`,
  `include/imgui`, and `include/fastnoise` separately, so vendored includes look like
  `#include "glad/glad.h"` / `#include "imgui.h"` / `#include "FastNoiseLite.h"`. Project
  headers are found relative to the including file in `src/`, e.g. `#include "chunk.h"`.

## `Constants` gotcha

`CHUNK_SIZE_X/Y/Z` and `CHUNK_SIZE` are `inline constexpr` in `constants.h` — usable in
constant expressions and array bounds. But `CHUNK_DISTANCE_*`, `FINAL_*`, and `NUM_*` are
`extern const` in the header and `extern constexpr` in `constants.cpp`; they are **not**
usable in a `static_assert` or array bound outside `constants.cpp`, and would produce
undefined-reference link errors if used as such. (The `static_assert(NUM_THREADS >= 2)` at
`src/constants.cpp:22` only compiles because the `constexpr` definition is in the same TU.)
The split was deliberate (commit `23aa29e`) to cut rebuild times — preserve it, or convert
all of them to `inline constexpr` deliberately rather than piecemeally. `CODE_REVIEW.md`
recommends the latter at this project size.

Also: `CHUNK_SIZE` is `100³` = 1,000,000, and `Chunk` holds
`std::array<Block, 1'000'000>` at **4 bytes/block** (`sizeof(Block)` is 4 because `BlockType`
is an `int`-backed enum, not 8) = **4 MB per chunk**, ~256 MB across the 64-chunk render list
(`FINAL_CHUNK_DISTANCE` = 8×1×8), and up to ~676 MB once the unload margin lets ~169 chunks
accumulate. Don't add per-block containers or copies casually.
`Chunk::GetBlocksArray()` returns that array **by value** (4 MB) — it is currently uncalled;
don't call it. The review recommends `enum class BlockType : std::uint8_t` stored directly,
which cuts this 4x.

`FINAL_CHUNK_DISTANCE` is for sizing/reserving only, not loop bounds — use
`m_chunkList.size()` or a `CHUNK_DISTANCE_*` product instead. `application.cpp:95` currently
uses it to size `doneVec`; that is equal to the render-list size by construction, but the
coupling is fragile (see `CODE_REVIEW.md`).

## Style

No `.clang-format` exists; match the surrounding code. Effectively clang-format LLVM with an
80-column limit: 2-space indent, braces on the same line, `m_camelCase` members,
`PascalCase` methods, `UPPER_SNAKE_CASE` in `namespace Constants`. The tree is currently
within 80 columns everywhere. `CODE_REVIEW.md` recommends adding the config and a CI format
check so this is enforceable.

Top-level `const` on return types was cleaned up. Three harmless leftovers remain — a
`const Mesh *const` parameter pair (`renderer.h:24`, `renderer.cpp:12`) and a
`const Chunk *const` local (`chunkmanager.cpp:60`). They mean nothing; don't propagate the
pattern in new code.

Known style red flags a reviewer will notice (fix before showing the repo):

- `const_cast` getters: `GameState::GetCamera()` (`gamestate.cpp:42-46`) and
  `Chunk::GetBlock` (`chunk.cpp:52-55`). The former is just `return m_camera;`.
- Deleted move constructors written as `const T&&` (`application.h:12`, `renderer.h:20`,
  `gamestate.h:11`, `window.h:14`). Use `T(T&&) = delete;`.
- `Window operator=(const Window&) = delete;` (`window.h:13`) is missing `&`.
- Top-level `const` on by-value parameters (`const int`, `const float`, `const glm::ivec3`)
  is ignored by the language; keep `const` only for references/pointers.

## Known dead code

Defined but unreferenced — don't wire it up or spend time "fixing" it unless asked. The
review recommends deleting all of it:

- `MesherBasic` (`mesher.h:23`; `ChunkManager` instantiates `MesherNaive`). It also reserves
  `CHUNK_SIZE * 6 * 6` vertices = ~432 MB if ever instantiated (`mesher.cpp:24-25`).
- `Chunk::GetBlocksArray` (`chunk.h:28`) — returns a 4 MB array by value.
- `Constants::FINAL_RENDER_DISTANCE` (`constants.cpp:16`).
- `Mesh::m_EBO` (`mesh.h:27`, `[[maybe_unused]]`).
- `Camera::ProcessMouseScroll` (`camera.cpp:56`).
- `Window::GetWindowPtr` (`window.cpp:19`).
- `assets/shaders/vertex.glsl` (the renderer uses `basic_vertex.glsl` + `fragment.glsl`).

Roughly 10% of `src/` is commented-out code; remove it. The review lists the exact line
ranges.
