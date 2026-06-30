cmake_minimum_required(VERSION 3.10)
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
  # include/desertedcraft
  vendor/glad
  vendor/imgui
)

if(WIN32)
  include(FetchContent)
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
  )
  FetchContent_MakeAvailable(glfw)
  target_link_libraries(desertedcraft PRIVATE
    glfw
  )

  target_include_directories(desertedcraft PRIVATE
    vendor/glm
  )

  # Make Visual Studio invoke in the correct directory
  if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_target_properties(desertedcraft PROPERTIES
      VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:desertedcraft>"
    )
  endif()

else()
  find_package(glfw3 REQUIRED)
  find_package(glm REQUIRED)

  target_link_libraries(desertedcraft PRIVATE
    glfw
    glm::glm
  )
endif()

add_custom_command(TARGET desertedcraft POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:desertedcraft>/assets
)