#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include <vertexArray.h>
#include <vertexBuffer.h>
#include <vertexBufferLayout.h>
#include <indexBuffer.h>
#include <shader.h>

#define MAX_BONE_INFLUENCE 4
using namespace std;

struct Vertex
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoords;
  glm::vec3 tangent;
  glm::vec3 bitangent;
  int m_BoneIDs[MAX_BONE_INFLUENCE];
  float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture
{
  unsigned int id;
  string type;
  string path;
};

class Mesh
{
public:
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  vector<Texture> textures;
  unsigned int VAO;

  Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
  void draw(Shader &shader);

private:
  unsigned int VBO, EBO;
  // VertexArray vao;
  // VertexBuffer vbo;
  // IndexBuffer ibo;
  // VertexBufferLayout layout;
  void setupMesh();
};