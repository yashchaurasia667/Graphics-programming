#pragma once

#include <glad/glad.h>

class IndexBuffer
{
private:
  unsigned int ID;
  unsigned int count;

public:
  IndexBuffer();
  IndexBuffer(const unsigned int *data, unsigned int count, GLenum usage);
  void setData(const unsigned int *data, unsigned int count, GLenum usage);
  void bind() const;
  void unbind() const;
  ~IndexBuffer();
};