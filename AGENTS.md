# AGENTS.md

C++20 voxel engine (OpenGL 3.3 / GLFW / glad / ImGui / oneTBB). ~1,700 lines across 26
files, all in `src/`.
There are **no tests, no CI, no linter config, and no `.clang-format`** in this repo. Don't
look for a `test`/`lint` command — there isn't one. The quality gate is the compiler:
warnings are errors, so `cmake --build` failing on a diagnostic is normal and expected.

## Warning flags are real, and the tree is clean

`CMakeLists.txt:99-107` applies `-Wall -Wextra -Werror -pedantic-errors` on
`CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"` (and `/W4 /WX /permissive-` on MSVC). The
condition is genuinely evaluated — it is not the quoted-string bug it used to be.

All 13 translation units in `src/` currently compile clean under those flags with `clang++`
and `g++`. If you touch a file, keep it that way; `-Werror` means any new warning breaks
the build.

For a fast single-file check without a full CMake configure:

```bash
clang++ -std=c++20 -fsyntax-only -Wall -Wextra -Werror -pedantic-errors \
    -Iinclude/glad -Iinclude/imgui -Iinclude/fastnoise -Iinclude src/<file>.cpp
```

No TBB include path is needed — oneTBB is system-installed at `/usr/include/oneapi`.
Only add `-I<builddir>/_deps/tbb-src/include` if you are on a machine where it isn't.

Two idioms are used deliberately to stay quiet under `-Werror`: `[[nodiscard]]` on
functions whose result must not be dropped (`ChunkManager::GetChunk`, `GetBlocksArray`,
the `CreateMesh` overrides, …) and `[[maybe_unused]]` on genuinely dead members.

## Build and run

A configured tree is present at `build/` (Ninja + `clang++` + Debug):

```bash
cmake --build build/                # incremental
./build/desertedcraft               # must be run FROM build/
```

Fresh configure (README documents the variants: `build/`, `build-gcc/Debug/`,
`build-clang/Debug/`, `build-msvc/`). Note this **clones and builds oneTBB from source**,
which is slow, and emits CMake deprecation warnings *from TBB's own* CMakeLists that are
not your problem:

```bash
cmake -B build/ -G Ninja -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
```

Gotchas:
- **Must be run from the directory containing `assets/`.** Shaders are loaded via relative
  paths (`renderer.cpp:9-10` hardcodes `./assets/shaders/...`). A `POST_BUILD` step
  (`CMakeLists.txt:109`) symlinks `assets/` next to the binary, which is why running from
  `build/` works.
- Needs a real display/GL context. It cannot be verified headless — build success is the
  only automated signal.
- The root `compile_commands.json` is a gitignored symlink to `out/Debug/…`, which **no
  longer exists** — it is broken. The live one is `build/compile_commands.json`. Repoint or
  delete the root symlink if a tool needs it.

## Threading and the OpenGL-context invariant

`Application::Run()` spawns `Constants::NUM_WORKERS` (9, of `NUM_THREADS` = 10) threads
running `ChunkManager::Dispatch`, which `pop` coords from `m_workQueue`, call
`GenerateChunk`, and `emplace` the result. `ChunkManager::Update()` runs on the **main
thread only** and does render-list rebuild, job dispatch, GPU upload, and unloading.

## Threading and the OpenGL-context invariant

`Application::Run()` spawns `Constants::NUM_WORKERS` (9, of `NUM_THREADS` = 10) threads
running `ChunkManager::Dispatch`, which `pop` coords from `m_workQueue`, call
`GenerateChunk`, and `emplace` the result. `ChunkManager::Update()` runs on the **main
thread only** and does render-list rebuild, job dispatch, GPU upload, and unloading.

`m_chunkMap` is a plain `std::unordered_map` shared between those threads. **As of commit
`41b708b` the lock scopes themselves are correct** — the four unsynchronized accesses
previously listed here (`GetChunk` unlocked, `Update`'s `find` before the lock, the
GPU-upload loop's post-release dereference, and `Unload`'s split `find`/`erase` scopes) have
all been fixed. There is no lock recursion either: the four `scoped_lock` scopes in `Update()`
are sequential rather than nested, and the `Unload()` call at the end is outside all of them.

Remaining problems, ranked. They are **not** the same set as the old stale list:

1. **`emplace` on a duplicate key destroys a `Chunk` on a worker thread** —
   `chunkmanager.cpp:224`. `unordered_map::emplace` on an existing key is a no-op and
   destroys the passed `unique_ptr` at the end of the full expression, on the worker. That
   runs `~Chunk` → `~Mesh` (`mesh.cpp:41-47`) → `glBindVertexArray` / `glBindBuffer` /
   `glDeleteBuffers` / `glDeleteVertexArrays` with no current context. Currently masked by
   bug 2 (duplicate coords are never queued), so it is latent — but **fixing 2 without fixing
   this converts a missing-chunk bug into a GL crash.** Check the `emplace` return value, or
   hand losers to a main-thread trash queue drained in `Update()`.
2. **`m_jobsQueuedList` is never cleared on unload, so chunks are permanently lost** —
   `chunkmanager.cpp:66-78`. `Unload()` erases from `m_chunkMap` only. The sole
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
4. **`renderList` is a reference into `m_chunkList` that survives `clear()` + refill** —
   `application.cpp:78` caches `const std::vector<glm::ivec3>&` to the member from
   `GetChunksRenderList()`. `Update()` does `clear()` + `BuildRenderList()` on that same
   vector (`chunkmanager.cpp:126-127`). No realloc happens only because the constructor
   `reserve`d `FINAL_CHUNK_DISTANCE` and the rebuild is exactly that size — an invariant
   nothing asserts. Push past capacity and the loop at `application.cpp:113` iterates freed
   memory.
5. **`doneVec` is sized by a reserve-only constant but indexed by `renderList.size()`** —
   `application.cpp:95` allocates `Constants::FINAL_CHUNK_DISTANCE` (64) elements, written at
   `application.cpp:125` for `i < renderList.size()`. Both are 64 today; divergence is an
   out-of-bounds write, and it is never resized across rebuilds. This is a violation of the
   "`FINAL_*` is reserve-only" rule below.
6. **The shared `Mesher` is called concurrently through non-const members** —
   `chunkmanager.cpp:114`. All 9 workers call `m_mesherPtr->CreateMesh(...)` on the single
   shared instance, and both `Mesher::CreateMesh` (`mesher.h:11`) and the protected
   `Mesher::BuildFace` (`mesher.h:18`) are non-const member functions. They touch no members
   today — `BuildFace` only reads the `const` globals in `cube.h:23,32,88` and appends to a
   caller-local vector — so it is benign, but nothing in the type system or a comment records
   the requirement. Marking both `const` makes it checkable.
7. **No join guard** — `application.cpp:87-179`. `workers` are joined at lines 177-179, which
   is only reached by falling out of the while loop. Any throw from lines 98-173 skips both
   `m_workQueue.abort()` and the joins, and `~vector<thread>` on joinable threads calls
   `std::terminate`. Needs a scope guard.
8. **GL teardown happens after `glfwTerminate()`** — `application.cpp:74`. The destructor body
   runs before members are destroyed, so `~GameState` → `~ChunkManager` → `~Chunk` → `~Mesh`
   issues `glDeleteBuffers`/`glDeleteVertexArrays` on a terminated context, as does
   `~Renderer` → `~Shader` (`shader.cpp:67-68`). `Window` has no destructor, so
   `glfwDestroyWindow` is never called explicitly either.

Invariants to preserve when touching this code:

- **OpenGL calls must stay on the main thread.** Worker threads have no GL context, but
  `~Mesh` calls `glDeleteBuffers`/`glDeleteVertexArrays` unconditionally, so destroying a
  `Chunk` off the main thread corrupts GL state. `Mesh::BufferData()` is main-thread-only by
  design. Note `~Mesh` also binds and deletes name 0, so even a zero-initialized handle is
  not a no-op.
- **Publication is correctly ordered.** A worker fully constructs the `Chunk`, then
  `emplace` takes `m_mutex` at `chunkmanager.cpp:223`; the main thread only ever reads
  published chunks under that same mutex. Do not "optimize" the lock away on either side.
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
- Shutting the tree down is a UB trap, not a race, and is unrelated to threads: `gamestate.h:23`
  declares `ChunkManager chunkManager;` ahead of `Camera m_camera;` at line 26, so members
  initialize in that order, and `ChunkManager::ChunkManager` (`chunkmanager.cpp:45`) calls
  `gamestate.GetPlayerChunkCoords()` → `GetCamera().Position` on a `Camera` whose lifetime
  has not begun. The mem-init list at `gamestate.cpp:7` matches the declaration order, so
  `-Wreorder-ctor` stays silent. It works only because `GameState` is `make_unique`'d onto
  fresh zero pages. Reorder the members, or initialize `m_currPlayerChunkCoords` lazily.


## Adding a source file

`CMakeLists.txt` lists sources explicitly in `add_executable(...)` — no globbing. New `.cpp`
files must be added there by hand or they will not be compiled.

## Layout / dependencies

- `include/` is **vendored third-party code**: `glad/`, `imgui/`, `fastnoise/`. Do not
  reformat, lint, or "fix" anything there.
- TBB is *not* in `include/`; it is `FetchContent`-ed into `build/_deps/tbb-src/`, and also
  installed system-wide.
- `include/` doubles as the include root, so includes look like `#include "chunk.h"` for
  project headers but `#include "glad/glad.h"` for vendored ones.

## `Constants` gotcha

`CHUNK_SIZE_X/Y/Z` and `CHUNK_SIZE` are `inline constexpr` in `constants.h` — usable in
constant expressions and array bounds. But `CHUNK_DISTANCE_*`, `FINAL_*`, and `NUM_*` are
`extern const` in the header and `extern constexpr` in `constants.cpp`; they are **not**
usable in a `static_assert` or array bound outside `constants.cpp`, and would produce
undefined-reference link errors if used as such. (The `static_assert(NUM_THREADS >= 2)` at
`src/constants.cpp:22` only compiles because the `constexpr` definition is in the same TU.)
The split was deliberate (commit `23aa29e`) to cut rebuild times — preserve it, or convert
all of them to `inline constexpr` deliberately rather than piecemeally.

Also: `CHUNK_SIZE` is `100³` = 1,000,000, and `Chunk` holds
`std::array<Block, 1'000'000>` at 8 bytes/block = **7.6 MB per chunk**, ~488 MB across the
64-chunk render list (`FINAL_CHUNK_DISTANCE` = 8×1×8). Don't add per-block containers or
copies casually. `Chunk::GetBlocksArray()` returns that array **by value** — it is currently
uncalled; don't call it.

`FINAL_CHUNK_DISTANCE` is for sizing/reserving only, not loop bounds — use
`m_chunkList.size()` or a `CHUNK_DISTANCE_*` product instead.

## Style

No config file exists; match the surrounding code. Effectively clang-format LLVM with an
80-column limit: 2-space indent, braces on the same line, `m_camelCase` members,
`PascalCase` methods, `UPPER_SNAKE_CASE` in `namespace Constants`. The tree is currently
within 80 columns everywhere.

Top-level `const` on return types was cleaned up. Three harmless leftovers remain — a
`const Mesh *const` parameter pair (`renderer.h:24`, `renderer.cpp:12`) and a
`const Chunk *const` local (`chunkmanager.cpp:60`). They mean nothing; don't propagate the
pattern in new code.

## Known dead code

Defined but unreferenced — don't wire it up or spend time "fixing" it unless asked:
`MesherBasic` (`mesher.h:23`; `ChunkManager` instantiates `MesherNaive`),
`Chunk::GetBlocksArray`, `Constants::FINAL_RENDER_DISTANCE`, `Mesh::m_EBO`
(`[[maybe_unused]]`), `Camera::ProcessMouseScroll`.

Roughly 10% of `src/` is commented-out code.
