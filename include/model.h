#pragma once

#include <shader.h>
#include <mesh.h>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;

class Model
{
public:
  vector<Texture> textures_loaded;
  vector<Mesh> meshes;
  string directory;
  bool gammaCorrection;

  Model(char *path)
  {
    loadModel(path);
  }
  void draw(Shader &shader);

private:
  void loadModel(string path);
  void processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
  vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
  unsigned int textureFromFile(const char *path, const string &directory, bool gamma);
};