cmake_minimum_required(VERSION 3.21)
project(cubemap LANGUAGES C CXX)

find_package(OpenGL REQUIRED)

set(INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
set(LIB     "${CMAKE_CURRENT_SOURCE_DIR}/../lib")
set(SRC     "${CMAKE_CURRENT_SOURCE_DIR}")

# ---- Library with actual compiled code ----
add_library(depends STATIC
  "${LIB}/glad.c"
  "${LIB}/shader.cpp"
  "${LIB}/vertexBuffer.cpp"
  "${LIB}/vertexBufferLayout.cpp"
  "${LIB}/indexBuffer.cpp"
  "${LIB}/vertexArray.cpp"
  "${LIB}/camera.cpp"
  "${LIB}/texture.cpp"
  "${LIB}/utils.cpp"
  "${LIB}/mesh.cpp"
  "${LIB}/model.cpp"
)

target_include_directories(depends
  PUBLIC
    "${INCLUDE}"
)

target_compile_options(depends
  PRIVATE
    -Wall
    -g
)

target_link_libraries(depends
  PUBLIC
    OpenGL::GL
    "${LIB}/libglfw3.a"
    "${LIB}/libassimp.a"
    "${LIB}/libzlibstatic.a"
    kernel32
    user32
    gdi32
)

# ---- Executable ----
add_executable(
  ${PROJECT_NAME}
    "${SRC}/main.cpp"
)

target_link_libraries(
  ${PROJECT_NAME}
    PRIVATE
      depends
)