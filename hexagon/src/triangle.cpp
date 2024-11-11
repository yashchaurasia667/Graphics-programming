#include "triangle.h"

Triangle::Triangle(vector<float> position, vector<int> colorIndices, int vertices, vector<int> elementIndices)
{
  vertex_count = vertices;

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // generate buffers for position and color
  VBOs.resize(2);

  // Vertex data
  glGenBuffers(1, &VBOs[0]);
  glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
  glBufferData(GL_ARRAY_BUFFER, position.size() * sizeof(float), position.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Color data
  glGenBuffers(1, &VBOs[1]);
  glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
  glBufferData(GL_ARRAY_BUFFER, colorIndices.size() * sizeof(int), colorIndices.data(), GL_STATIC_DRAW);

  glVertexAttribIPointer(1, 1, GL_INT, sizeof(int), (void *)0);
  glEnableVertexAttribArray(1);

  // Element buffer
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementIndices.size() * sizeof(int), elementIndices.data(), GL_STATIC_DRAW);
}

void Triangle::draw()
{
  glBindVertexArray(VAO);
  // glDrawArrays(GL_TRIANGLES, 0, vertex_count);
  glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, 0);
}

Triangle::~Triangle()
{
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(VBOs.size(), VBOs.data());
  glDeleteBuffers(1, &EBO);
}