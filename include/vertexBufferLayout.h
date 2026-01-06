#pragma once
#include <vector>
#include <glad/glad.h>
#include "utils.h"

struct VertexBufferElement
{
  unsigned int type;
  unsigned int count;
  unsigned char normalized;

  static unsigned int getSizeOfType(unsigned int type)
  {
    switch (type)
    {
    case GL_FLOAT:
      return 4;
    case GL_UNSIGNED_INT:
      return 4;
    case GL_UNSIGNED_BYTE:
      return 1;
    }

    ASSERT(false);
    return 0;
  }
};

class VertexBufferLayout
{
private:
  std::vector<VertexBufferElement> elements;
  unsigned int stride;

public:
  VertexBufferLayout() : stride(0) {};

  template <typename T>
  void Push(int count)
  {
    static_assert(false);
  }

  template <>
  void Push<float>(int count)
  {
    struct VertexBufferElement v = {GL_FLOAT, count, GL_FALSE};
    elements.push_back(v);
    stride += VertexBufferElement::getSizeOfType(GL_FLOAT);
  }

  template <>
  void Push<unsigned int>(int count)
  {
    struct VertexBufferElement v = {GL_UNSIGNED_INT, count, GL_FALSE};
    elements.push_back(v);
    stride += VertexBufferElement::getSizeOfType(GL_UNSIGNED_INT);
  }

  template <>
  void Push<unsigned char>(int count)
  {
    struct VertexBufferElement v = {GL_UNSIGNED_BYTE, count, GL_TRUE};
    elements.push_back(v);
    stride += VertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE);
  }

  inline const std::vector<VertexBufferElement> *getElements() const { return &elements; }
  inline unsigned int getStride() const { return stride; }
};