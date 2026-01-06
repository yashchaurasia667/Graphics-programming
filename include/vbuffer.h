#pragma once

#include <glad/glad.h>

class VBuffer
{
private:
  unsigned int ID;

public:
  VBuffer();
  VBuffer(const float *data, unsigned int count, GLenum usage);
  void setData(const float *data, unsigned int count, GLenum usage);
  void bind() const;
  void unbind() const;
  ~VBuffer();
};