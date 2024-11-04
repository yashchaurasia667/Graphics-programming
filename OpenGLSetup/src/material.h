#include "config.h"

class Material
{
public:
  Material(const char *fileName);
  ~Material();
  void use();

private:
  unsigned int texture;
};