#include "vertexArray.h"
#include "utils.h"

VertexArray::VertexArray()
{
  GLcall(glGenVertexArrays(1, &this->ID));
  GLcall(glBindVertexArray(this->ID));
}

VertexArray::~VertexArray()
{
  GLcall(glDeleteVertexArrays(1, &this->ID));
}

void VertexArray::addBuffer(const VertexBuffer &vb, const VertexBufferLayout &layout)
{
  bind();
  vb.bind();
  const auto *elements = layout.getElements();
  unsigned int offset = 0;

  for (unsigned int i = 0; i < elements->size(); i++)
  {
    const auto element = (*elements)[i];
    GLcall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(), (void *)(uintptr_t)offset));
    GLcall(glEnableVertexAttribArray(i));
    offset += element.count * VertexBufferElement::getSizeOfType(element.type);
  }
}

void VertexArray::bind() const
{
  GLcall(glBindVertexArray(this->ID));
}

void VertexArray::unbind() const
{
  GLcall(glBindVertexArray(0));
}