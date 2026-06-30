# desertedcraft

## Requirements:

### Windows
- CMake

Libraries such as `GLFW`, `glm`, and `glad` are bundled in. For `GLFW`, libraries for both `MSVC` and `MinGW-w64` are included

### Linux
- CMake
- glfw
- glm

`glfw` and `glm` should be installed by your system's package manager; CMake will look for these libraries on your system.

## Building

To build,

```bash
cmake -B build/ -S ./ # Optionally add preferred build system; ex. "-G Ninja"
cmake --build build/
```

and then run the executable in its output directory.