# desertedcraft

## Requirements

### Windows
- CMake
- Any C++17 compiler

`GLFW` and `glm` are automatically pulled in by CMake. `glad` is bundled in.

### Linux
- CMake
- Any C++17 compiler
- glfw
- glm

`glfw` and `glm` should be installed by your system's package manager; CMake will look for these libraries on your system.

## Building

To build,

```bash
git clone https://github.com/desertedman/desertedcraft
cd desertedcraft
# For faster builds: 
# cmake -B build/ -S ./ -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake -B build/ -S ./
cmake --build build/
```

and then run the executable in its output directory.
