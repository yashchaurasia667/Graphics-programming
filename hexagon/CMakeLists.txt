cmake_minimum_required(VERSION 3.10)
project(hexagon VERSION 1.0)

find_package(OpenGL REQUIRED)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include")
set(GLFW_LIB "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib/libglfw3.a")

add_executable(
  hex
  src/main.cpp
  src/glad.c
  src/utils.cpp
  src/triangle.cpp
)

target_include_directories(hex PRIVATE ${INCLUDE_DIR})
target_link_libraries(hex ${GLFW_LIB} OpenGL::GL)