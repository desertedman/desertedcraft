cmake_minimum_required(VERSION 3.11) # Use fetchcontent
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

find_package(glfw3 QUIET)
find_package(glm QUIET)

if(glfw3_FOUND)
  message(STATUS "glfw found!")
else()
  message(STATUS "glfw not found. Building from source...")
  include(fetchcontent)
  fetchcontent_declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
  )
  fetchcontent_makeavailable(glfw)
endif()

if(glm_FOUND)
  message(STATUS "glm found!")
else()
  message(STATUS "glm not found. Building from source...")
  include(fetchcontent)
  fetchcontent_declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
  )
  fetchcontent_makeavailable(glm)
endif()

target_link_libraries(desertedcraft PRIVATE
  glfw
  glm::glm
)

if(WIN32)
  # Make Visual Studio invoke in the correct directory
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_target_properties(desertedcraft PROPERTIES
      VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:desertedcraft>"
    )
  endif()
endif()

add_custom_command(TARGET desertedcraft POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:desertedcraft>/assets
)
