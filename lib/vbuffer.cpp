#include "VBuffer.h"
#include <utils.h>

VBuffer::VBuffer()
{
  this->ID = 0;
  GLcall(glGenBuffers(1, &this->ID));
  GLcall(glBindBuffer(GL_ARRAY_BUFFER, ID));
}

VBuffer::VBuffer(const float *data, unsigned int count, GLenum usage) : VBuffer()
{
  GLcall(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), data, usage));
}

void VBuffer::setData(const float *data, unsigned int count, GLenum usage)
{
  GLcall(glBufferData(GL_ARRAY_BUFFER, count * sizeof(float), data, usage));
}

void VBuffer::bind() const
{
  glBindBuffer(GL_ARRAY_BUFFER, this->ID);
}

void VBuffer::unbind() const
{
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VBuffer::~VBuffer()
{
  GLcall(glDeleteBuffers(1, &this->ID));
}
