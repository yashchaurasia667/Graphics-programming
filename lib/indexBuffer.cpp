#include "indexBuffer.h"
#include <utils.h>

IndexBuffer::IndexBuffer()
{
  this->ID = -1;
  GLcall(glGenBuffers(1, &this->ID));
  GLcall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ID));
}

IndexBuffer::IndexBuffer(const unsigned int *data, unsigned int count, GLenum usage) : IndexBuffer()
{
  GLcall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, usage));
}

void IndexBuffer::setData(const unsigned int *data, unsigned int count, GLenum usage)
{
  GLcall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, usage));
}

void IndexBuffer::bind() const 
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ID);
}

void IndexBuffer::unbind() const
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer()
{
  GLcall(glDeleteBuffers(1, &this->ID));
}
