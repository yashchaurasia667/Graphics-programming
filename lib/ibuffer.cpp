#include "IBuffer.h"
#include <utils.h>

IBuffer::IBuffer()
{
  this->ID = -1;
  GLcall(glGenBuffers(1, &this->ID));
  GLcall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ID));
}

IBuffer::IBuffer(const unsigned int *data, unsigned int count, GLenum usage) : IBuffer()
{
  GLcall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, usage));
}

void IBuffer::setData(const unsigned int *data, unsigned int count, GLenum usage)
{
  GLcall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, usage));
}

void IBuffer::bind() const 
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ID);
}

void IBuffer::unbind() const
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IBuffer::~IBuffer()
{
  GLcall(glDeleteBuffers(1, &this->ID));
}
