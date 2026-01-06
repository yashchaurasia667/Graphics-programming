#pragma once

#include <glad/glad.h>

class VertexBuffer
{
private:
  unsigned int ID;

public:
  VertexBuffer();
  VertexBuffer(const float *data, unsigned int count, GLenum usage);
  ~VertexBuffer();

  void setData(const float *data, unsigned int count, GLenum usage);
  void bind() const;
  void unbind() const;
};