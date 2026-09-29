cmake_minimum_required(VERSION 3.11) # Use fetchcontent
project(desertedcraft VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
  src/application.cpp
  src/shader.cpp
  src/block.cpp
  src/renderer.cpp
  src/camera.cpp
  src/gamestate.cpp
  src/chunk.cpp
  src/mesh.cpp
  src/mesher.cpp
  src/window.cpp
  src/chunkmanager.cpp
  src/constants.cpp

  include/glad/glad/glad.c
  include/imgui/imgui.cpp
  include/imgui/imgui_demo.cpp
  include/imgui/imgui_draw.cpp
  include/imgui/imgui_impl_glfw.cpp
  include/imgui/imgui_impl_opengl3.cpp
  include/imgui/imgui_tables.cpp
  include/imgui/imgui_widgets.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
  # include/desertedcraft
  include/glad
  include/imgui
  include/fastnoise
)

find_package(glfw3 QUIET)
find_package(glm QUIET)
find_package(TBB QUIET)
include(FetchContent)

if(glfw3_FOUND)
  message(STATUS "glfw found!")
else()
  message(STATUS "glfw not found. Building from source...")
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
  )
  FetchContent_MakeAvailable(glfw)
endif()

if(glm_FOUND)
  message(STATUS "glm found!")
else()
  message(STATUS "glm not found. Building from source...")
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
  )
  FetchContent_MakeAvailable(glm)
endif()

if(TBB_FOUND)
  message(STATUS "oneTBB found!")
else()
  message(STATUS "oneTBB not found. Building from source...")
  FetchContent_Declare(
  TBB
  GIT_REPOSITORY https://github.com/uxlfoundation/oneTBB.git
  GIT_TAG v2023.1.0
)
  set(TBB_TEST OFF CACHE BOOL "" FORCE)
  set(TBB_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(TBB_STRICT OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(TBB)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
  glfw
  glm::glm
  TBB::tbb
)

# Make Visual Studio invoke in the correct directory
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
  )

  target_compile_options(${PROJECT_NAME} PRIVATE
    /W4
    /WX
    /permissive-
  )

  # Strict errors
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Werror
    -pedantic-errors
  )
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
