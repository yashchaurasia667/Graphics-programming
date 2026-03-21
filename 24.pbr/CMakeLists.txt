cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ssao LANGUAGES C CXX)
find_package(OpenGL REQUIRED)

set(USR_INCLUDE "/usr/local/include/")
set(USR_SRC     "/usr/local/src")
set(USR_LIB     "/usr/local/lib")
set(INCLUDE     "${CMAKE_CURRENT_SOURCE_DIR}/../include")
set(LIB         "${CMAKE_CURRENT_SOURCE_DIR}/../lib")
set(SRC         "${CMAKE_CURRENT_SOURCE_DIR}")

# ---- Library with actual compiled code ----
add_library(depends STATIC
  "${USR_SRC}/glad.c"
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
  "${USR_INCLUDE}"
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
  "${USR_LIB}/libglfw3.a"
  "${USR_LIB}/libassimp.a"
  minizip dl z pthread
  X11 Xrandr Xi
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
