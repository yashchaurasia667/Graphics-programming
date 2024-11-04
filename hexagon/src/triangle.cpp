#include "triangle.h"

Triangle::Triangle()
{
  std::vector<float> position = {
      -1.0f, -1.0f, 0.0f,
      1.0f, -1.0f, 0.0f,
      0.0f, 1.0f, 0.0f};

  std::vector<int> colorIndices = {0, 1, 2};
  vertex_count = 3;

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
}

void Triangle::draw()
{
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, vertex_count);
}

Triangle::~Triangle()
{
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(VBOs.size(), VBOs.data());
}