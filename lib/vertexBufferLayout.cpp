#include "vertexBufferLayout.h"

template <typename T>
void VertexBufferLayout::push(unsigned int count)
{
  static_assert(false);
}

template <>
void VertexBufferLayout::push<float>(unsigned int count)
{
  struct VertexBufferElement v = {GL_FLOAT, count, GL_FALSE};
  elements.push_back(v);
  stride += count * VertexBufferElement::getSizeOfType(GL_FLOAT);
}

template <>
void VertexBufferLayout::push<unsigned int>(unsigned int count)
{
  struct VertexBufferElement v = {GL_UNSIGNED_INT, count, GL_FALSE};
  elements.push_back(v);
  stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_INT);
}

template <>
void VertexBufferLayout::push<unsigned char>(unsigned int count)
{
  struct VertexBufferElement v = {GL_UNSIGNED_BYTE, count, GL_TRUE};
  elements.push_back(v);
  stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE);
}
