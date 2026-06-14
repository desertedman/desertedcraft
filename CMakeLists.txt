cmake_minimum_required(VERSION 3.5.0)
project(desertedcraft VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
    desertedcraft
    src/main.cpp
    src/application.cpp
    src/shader.cpp
    src/block.cpp
    src/renderer.cpp
    src/camera.cpp
    src/gamestate.cpp
    src/chunk.cpp
    src/drawable.cpp
    src/mesher.cpp
    src/mesher_basic.cpp
    src/mesher_naive.cpp
    src/window.cpp
    src/chunkmanager.cpp

    vendor/glad/glad/glad.c
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_demo.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_impl_glfw.cpp
    vendor/imgui/imgui_impl_opengl3.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
)

target_include_directories(desertedcraft PRIVATE
  include/desertedcraft
  vendor/glad
  vendor/imgui
)

if(WIN32)
  target_include_directories(desertedcraft PRIVATE
    vendor/glm
    vendor/glfw
  )

  target_link_libraries(desertedcraft PRIVATE
    ${CMAKE_SOURCE_DIR}/vendor/glfw/glfw3.lib
  )

else()
  find_package(glfw3 REQUIRED)
  find_package(glm REQUIRED)

  target_link_libraries(desertedcraft PRIVATE
    glfw
    glm::glm
  )
endif()
