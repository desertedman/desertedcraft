# desertedcraft

## Requirements

### Windows
- CMake
- Any C++20 compiler (msvc compiles faster)
- Ninja (optional)

`GLFW` and `glm` are automatically pulled in by CMake.

### Linux
- CMake
- Any C++20 compiler (clang compiles faster)
- Ninja (optional)
- glfw
- glm

`glfw` and `glm` should be installed by your system's package manager; CMake will look for these libraries on your system.

## Building

To build,

```bash
git clone https://github.com/desertedman/desertedcraft
cd desertedcraft
cmake -B build/ -S ./
cmake --build build/
```

and then run the executable in its output directory.

### Examples

These are CMake commands that I frequently use for reference.

Windows:

```bash
# GCC
# Debug builds do not currently compile with GCC on Windows; unknown why.
cmake -B .\build-gcc\Debug\ -G Ninja -DCMAKE_BUILD_TYPE=Debug --fresh

# Release builds compile fine
cmake -B .\build-gcc\RelWithDebInfo\ -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo --fresh

# MSVC
cmake -B .\build-msvc\ 
cmake --build .\build-msvc\ --config Debug # or your preferred target here...
```

Linux:

```bash
# GCC
cmake -B .\build-gcc\Debug\ -G Ninja -DCMAKE_BUILD_TYPE=Debug --fresh

# Clang (my preferred method for super fast builds :) )
cmake -B .\build-clang\Debug\ -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_C_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug --fresh
```

## Libraries Used

- glfw
- glm
- glad
- imgui
- oneTBB
- FastNoise

## Addendum

If you're interested in seeing how long initial chunk generation takes across a number of thread + platform + compiler combinations, I've [benchmarked performance here](https://docs.google.com/spreadsheets/d/1kYBDrMohcR3O82KtVJ3cHqd4ai3uEZvPBXTEk7mz184/edit?usp=sharing).


Interestingly, MSVC on Windows performs slowest, while it seems like GCC slightly pulls ahead of Clang on Linux.
