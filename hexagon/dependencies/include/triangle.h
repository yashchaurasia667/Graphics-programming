#pragma once
#include "utils.h"

using namespace std;

class Triangle
{
public:
  Triangle(vector<float> position, vector<int> colorIndices, int vertices, vector<int> elementIndices);
  void draw();
  ~Triangle();

private:
  unsigned int EBO, VAO, vertex_count;
  std::vector<unsigned int> VBOs;
};