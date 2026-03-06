#pragma once

#include <glad/glad.h>
#include <iostream>

#ifdef _MSC_VER
  #define ASSERT(x) if (!(x)) __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
  #define ASSERT(x) if (!(x)) __builtin_trap()
#else
  #include <signal.h>
  #define ASSERT(x) if (!(x)) raise(SIGTRAP)
#endif

#define GLcall(x) \
  GLclearError(); \
  x;              \
  ASSERT(GLlogCall(#x, __FILE__, __LINE__))


void GLclearError();
bool GLlogCall(const char *function, const char *file, int line);
