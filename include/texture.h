#pragma once

class Texture
{
private:
  int width, height, nrChannels;

public:
  unsigned int ID;
  Texture(const char *path);
  Texture(const char *path, unsigned int wrap_param);
  ~Texture();

  void bind();
  void unbind();
};