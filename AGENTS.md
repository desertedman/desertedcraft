# AGENTS.md

C++20 voxel engine (OpenGL 3.3 / GLFW / glad / ImGui / oneTBB). ~1,700 lines, all in `src/`.
There are **no tests, no CI, no linter config, and no `.clang-format`** in this repo. Don't
look for a `test`/`lint` command — there isn't one. The closest thing to a quality gate is
the manual warning check below, and it should be run on any file you touch.

## The `-Wall -Wextra` flags are never actually applied

`CMakeLists.txt` has `elseif("GNU|Clang")` — a quoted string, which CMake evaluates as
false, always. So `target_compile_options(... -Wall -Wextra -pedantic-errors)` has never run.
The code is **not** warning-clean; there are ~30 warnings, including three `-Wreorder`
(member-init lists in `Camera`, `Window`, `Renderer` don't match declaration order).

**Never assume a build is warning-clean because it was quiet.** Verify explicitly:

```bash
g++ -std=c++20 -fsyntax-only -Wall -Wextra \
    -Iinclude/glad -Iinclude/imgui -Iinclude/fastnoise \
    -Iout/Debug/_deps/tbb-src/include src/<file>.cpp
```

(the TBB include path comes from the configured build tree, not from `include/`)

If asked to fix warnings, fixing the `elseif` is the first step — then expect fallout.

## Build and run

An existing configured tree is usually present at `out/Debug/` (Ninja + `clang++` + Debug):

```bash
cmake --build out/Debug/            # incremental
./out/Debug/desertedcraft           # must be run FROM out/Debug/
```

Fresh configure (README has the variants) — note this **clones and builds oneTBB from
source**, which is slow, and emits CMake deprecation warnings *from TBB's own* CMakeLists
that are not your problem:

```bash
cmake -B out/Debug/ -G Ninja -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
```

Gotchas:
- **Must be run from the directory containing `assets/`.** Shaders are loaded via relative
  paths (`renderer.cpp` hardcodes `./assets/shaders/...`). A `POST_BUILD` step copies
  `assets/` next to the binary, which is why running from `out/Debug/` works by accident.
- Needs a real display/GL context. It cannot be verified headless — build success is the
  only automated signal.
- `compile_commands.json` at the repo root is a gitignored symlink into `out/Debug/`.

## Threading and the OpenGL-context invariant

This is the part most likely to be broken by a careless edit:

- `Application::Run()` spawns `Constants::NUM_WORKERS` threads running
  `ChunkManager::Dispatch`, which pull coords from `m_workQueue` and call `GenerateChunk`.
- `ChunkManager::Update()` runs on the **main thread only** and does render-list rebuild,
  job dispatch, GPU upload, and unloading.
- Intended invariant: **`m_chunkMap` is guarded by `m_mutex`.** It is currently *not* held
  consistently — `Update()` iterates the map unlocked while workers `emplace` into it, and
  `Unload()` uses an iterator across a lock release. Treat any new map access as
  "must be inside one `scoped_lock` scope."
- **OpenGL calls must stay on the main thread.** Worker threads have no GL context, but
  `~Mesh` calls `glDeleteBuffers`/`glDeleteVertexArrays`, so destroying a `Chunk` off the
  main thread corrupts GL state. `Mesh::BufferData()` is main-thread-only by design.
- `m_workQueue.pop()` **blocks**, so `while (running)` is not a shutdown mechanism — the
  `running` flag is effectively decorative. Shutdown works only via
  `chunkManager.m_workQueue.abort()` throwing `user_abort`.
- `m_workQueue` is a public member and `Application` aborts it directly; `Coordinator` is
  declared in `chunkmanager.h` but **never defined or called** (abandoned design).

## Adding a source file

`CMakeLists.txt` lists sources explicitly in `add_executable(...)` — no globbing. New `.cpp`
files must be added there by hand or they will not be compiled.

## Layout / dependencies

- `include/` is **vendored third-party code**: `glad/`, `imgui/`, `fastnoise/`. Do not
  reformat, lint, or "fix" anything there.
- TBB is *not* in `include/`; it's `FetchContent`-ed into `out/Debug/_deps/tbb-src/`.
- `include/` doubles as the include root, so includes look like `#include "chunk.h"` for
  project headers but `#include "glad/glad.h"` for vendored ones.

## `Constants` gotcha

`CHUNK_SIZE_X/Y/Z` and `CHUNK_SIZE` are `inline constexpr` in `constants.h` — usable in
constant expressions and array bounds. But `CHUNK_DISTANCE_*`, `FINAL_*`, and `NUM_*` are
`extern const` in the header and defined in `constants.cpp`; they are **not** usable in a
`static_assert` or array bound and produce undefined-reference link errors. The split was
deliberate (commit `23aa29e`) to cut rebuild times — preserve it, or convert all of them to
`inline constexpr` deliberately rather than piecemeal.

Also: `CHUNK_SIZE` is `100³` = 1,000,000, and `Chunk` holds
`std::array<Block, 1'000'000>` at 8 bytes/block = **7.6 MB per chunk**, ~488 MB across the
64-chunk render list. Don't add per-block containers or copies casually.
`Chunk::GetBlocksArray()` returns that array **by value** — it is currently uncalled; don't
call it.

## Style

No config file exists; match the surrounding code. Effectively clang-format LLVM with an
80-column limit: 2-space indent, braces on the same line, `m_camelCase` members,
`PascalCase` methods, `UPPER_SNAKE_CASE` in `namespace Constants`. One known outlier
exceeds 80 columns at `src/chunk.cpp:48`.

Note the codebase contains several top-level `const` on return values and pointers
(`const glm::ivec3 Foo()`, `const Chunk *const GetChunk()`), which triggers
`-Wignored-qualifiers` and is meaningless. Don't propagate the pattern in new code.

## Known dead code

Defined but unreferenced — don't wire it up or spend time "fixing" it unless asked:
`ChunkManager::Coordinator`, `MesherBasic`, `Chunk::GetBlocksArray`,
`Constants::FINAL_RENDER_DISTANCE`, `Mesh::m_EBO`, `Block::isActive`,
`Window::SetCursorMode`, `Camera::ProcessMouseScroll`.

Roughly 10% of `src/` is commented-out code, and `src/cube.h` is **missing `#pragma once`**
(double-inclusion is a hard error — it compiles only by include-order luck).
