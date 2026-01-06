#include "vertexBuffer.h"
#include <utils.h>

VertexBuffer::VertexBuffer()
{
  this->ID = 0;
  GLcall(glGenBuffers(1, &this->ID));
  GLcall(glBindBuffer(GL_ARRAY_BUFFER, ID));
}

VertexBuffer::VertexBuffer(const float *data, unsigned int count, GLenum usage) : VertexBuffer()
{
  GLcall(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), data, usage));
}

void VertexBuffer::setData(const float *data, unsigned int count, GLenum usage)
{
  GLcall(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), data, usage));
}

void VertexBuffer::bind() const
{
  glBindBuffer(GL_ARRAY_BUFFER, this->ID);
}

void VertexBuffer::unbind() const
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::~VertexBuffer()
{
  GLcall(glDeleteBuffers(1, &this->ID));
}
