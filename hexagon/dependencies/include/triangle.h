#pragma once
#include "utils.h"

class Triangle
{
public:
  Triangle();
  void draw();
  ~Triangle();

private:
  unsigned int VAO, vertex_count;
  std::vector<unsigned int> VBOs;
};