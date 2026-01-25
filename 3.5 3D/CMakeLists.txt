cmake_minimum_required(VERSION 4.1.2)
project(3d)

find_package(OpenGL REQUIRED)

add_library(depends INTERFACE)
target_sources(depends INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../lib/shader.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/../lib/glad.c")
target_include_directories(depends INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_link_libraries(depends INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../lib/libglfw3.a" OpenGL::GL kernel32 user32 gdi32)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE depends)