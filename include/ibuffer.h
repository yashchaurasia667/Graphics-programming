#pragma once

#include <glad/glad.h>

class IBuffer
{
private:
  unsigned int ID;
  unsigned int count;

public:
  IBuffer();
  IBuffer(const unsigned int *data, unsigned int count, GLenum usage);
  void setData(const unsigned int *data, unsigned int count, GLenum usage);
  void bind() const;
  void unbind() const;
  ~IBuffer();
};