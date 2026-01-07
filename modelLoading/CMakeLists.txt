cmake_minimum_required(VERSION 4.1.2)
project(modelLoading)

find_package(OpenGL REQUIRED)

set(INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
set(LIB "${CMAKE_CURRENT_SOURCE_DIR}/../lib")
set(CMAKE_CXX_FLAGS "-Wall -g")

add_library(depends INTERFACE)
target_include_directories(depends INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_sources(depends INTERFACE "${LIB}/shader.cpp" "${LIB}/vertexBufferLayout.cpp" "${LIB}/glad.c" "${LIB}/utils.cpp" "${LIB}/camera.cpp" "${LIB}/vertexBuffer.cpp" "${LIB}/vertexArray.cpp" "${LIB}/indexBuffer.cpp" "${LIB}/mesh.cpp" "${LIB}/model.cpp")

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(depends INTERFACE "${LIB}/libglfw3.a" "${LIB}/libassimp.a" "${LIB}/libzlibstatic.a" OpenGL::GL kernel32 user32 gdi32)
target_link_libraries(${PROJECT_NAME} PRIVATE depends)
