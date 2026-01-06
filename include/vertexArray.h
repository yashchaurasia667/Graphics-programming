#pragma once

#include "vertexBuffer.h"
#include "vertexBufferLayout.h"

class VertexArray
{
private:
  unsigned int ID;

public:
  VertexArray();
  ~VertexArray();

  void addBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout);
  void bind() const;
  void unbind() const;
};