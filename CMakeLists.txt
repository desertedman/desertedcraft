cmake_minimum_required(VERSION 3.5.0)
project(desertedcraft VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
    desertedcraft src/glad.c src/main.cpp src/application.cpp src/shader.cpp
    src/block.cpp src/renderer.cpp src/camera.cpp src/gamestate.cpp
)
target_include_directories(desertedcraft PRIVATE include)
target_link_libraries(desertedcraft PRIVATE glfw)
