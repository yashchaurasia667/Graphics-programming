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


void GLclearError();
bool GLlogCall(const char *function, const char *file, int line);