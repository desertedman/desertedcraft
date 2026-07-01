# desertedcraft

## Requirements

### Windows
- CMake
- Any C++17 compiler

Libraries such as `GLFW`, `glm`, and `glad` are bundled in. `GLFW` automatically builds against MSVC/MinGW-w64.

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
cmake -B build/ -S ./ # Optionally add preferred build system; ex. "-G Ninja"
cmake --build build/
```

and then run the executable in its output directory.