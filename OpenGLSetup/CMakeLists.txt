cmake_minimum_required(VERSION 3.10)
project(main VERSION 1.0)

find_package(OpenGL REQUIRED)

set(GLFW_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include")
set(GLFW_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib/libglfw3.a")

add_executable(main
  src/config.h
  src/config.cpp
  src/main.cpp 
  src/glad.c 
  src/triangle_mesh.cpp 
  src/material.cpp
)

target_include_directories(main PRIVATE ${GLFW_INCLUDE_DIRS})

target_link_libraries(main ${GLFW_LIBRARIES} OpenGL::GL)