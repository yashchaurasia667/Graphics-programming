#pragma once
#include <glad/glad.h>
#include <iostream>

#define ASSERT(x) \
  if (!x)         \
    __debugbreak();
#define GLcall(x) \
  GLclearError(); \
  x;              \
  ASSERT(GLlogCall(#x, __FILE__, __LINE__))

void GLclearError()
{
  while (glGetError() != GL_NO_ERROR)
    ;
}

bool GLlogCall(const char *function, const char *file, int line)
{
  while (GLenum error = glGetError())
  {
    std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << std::endl;
    return false;
  }
  return true;
}