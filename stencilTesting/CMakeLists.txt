cmake_minimum_required(VERSION 4.1.2)
project(stencilTesting)

find_package(OpenGL REQUIRED)

set(INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
set(LIB "${CMAKE_CURRENT_SOURCE_DIR}/../lib")

add_library(depends INTERFACE)
target_sources(depends INTERFACE "${LIB}/shader.cpp" "${LIB}/camera.cpp" "${LIB}/glad.c" "${LIB}/vertexArray.cpp" "${LIB}/vertexBuffer.cpp" "${LIB}/vertexBufferLayout.cpp" "${LIB}/indexBuffer.cpp" "${LIB}/texture.cpp" "${LIB}/utils.cpp")
target_include_directories(depends INTERFACE "${INCLUDE}")
target_link_libraries(depends INTERFACE "${LIB}/libglfw3.a" OpenGL::GL kernel32 user32 gdi32)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE depends)