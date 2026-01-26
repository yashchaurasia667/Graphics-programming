#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <map>

#include <utils.h>
#include <shader.h>
#include <camera.h>
#include <texture.h>
#include <vertexArray.h>
#include <vertexBuffer.h>
#include <vertexBufferLayout.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;

bool firstMouse = true;
float sensitivity = 0.1f, speed = 2.5f;
float deltaTime = 0.0f, lastFrame = 0.0f;
Camera camera;

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
unsigned int loadCubemap(std::vector<std::string> faces);

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "TITLE", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create a GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to load OpenGL function pointers" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glEnable(GL_DEPTH_TEST);

  float skyboxVertices[] = {
      // positions
      -1.0f, 1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, 1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f,

      -1.0f, -1.0f, 1.0f,
      -1.0f, -1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f,
      -1.0f, 1.0f, -1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, -1.0f, 1.0f,

      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,

      -1.0f, -1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, -1.0f, 1.0f,
      -1.0f, -1.0f, 1.0f,

      -1.0f, 1.0f, -1.0f,
      1.0f, 1.0f, -1.0f,
      1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, 1.0f,
      -1.0f, 1.0f, -1.0f,

      -1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f, 1.0f,
      1.0f, -1.0f, -1.0f,
      1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f, 1.0f,
      1.0f, -1.0f, 1.0f};

  float cubeVertices[] = {
      // positions          // normals           // texture coords
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
      0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
      0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
      0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
      -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
      0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

      -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

      0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
      0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
      0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
      0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
      0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
      0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
      0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
      -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

  {
    Shader def("../shaders/cubemaps.vs", "../shaders/cubemaps.fs");
    Shader cube("../shaders/default.vs", "../shaders/default.fs");

    VertexArray skyboxVao;
    VertexBuffer vbo(skyboxVertices, 36 * 3, GL_STATIC_DRAW);
    VertexBufferLayout layout;

    layout.push<float>(3);
    skyboxVao.addBuffer(vbo, layout);

    std::vector<std::string> textures_faces;
    textures_faces.push_back("../../resources/skybox/right.jpg");
    textures_faces.push_back("../../resources/skybox/left.jpg");
    textures_faces.push_back("../../resources/skybox/top.jpg");
    textures_faces.push_back("../../resources/skybox/bottom.jpg");
    textures_faces.push_back("../../resources/skybox/front.jpg");
    textures_faces.push_back("../../resources/skybox/back.jpg");
    unsigned int cubemapTex = loadCubemap(textures_faces);

    VertexArray cubeVao;
    VertexBuffer cubeVbo(cubeVertices, 36 * 8, GL_STATIC_DRAW);
    VertexBufferLayout cubeLayout;

    cubeLayout.push<float>(3);
    cubeLayout.push<float>(3);
    cubeLayout.push<float>(2);
    cubeVao.addBuffer(cubeVbo, cubeLayout);

    while (!glfwWindowShouldClose(window))
    {
      glfwPollEvents();

      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;
      processInput(window);

      glClearColor(0.5f, 0.3f, 0.2f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 view = camera.get_view_matrix();
      glm::mat4 projection = glm::perspective(camera.get_fov(), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

      model = glm::scale(model, glm::vec3(0.5f));
      cube.use();
      cube.setMat4("model", model);
      cube.setMat4("view", view);
      cube.setMat4("projection", projection);
      cube.setVec3("cameraPos", camera.get_pos());

      cubeVao.bind();
      GLcall(glActiveTexture(GL_TEXTURE0));
      GLcall(glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex));
      cube.setInt("skybox", 0);
      GLcall(glDrawArrays(GL_TRIANGLES, 0, 36));

      // glDepthMask(GL_FALSE);
      GLcall(glDepthFunc(GL_LEQUAL));
      def.use();
      def.setMat4("view", glm::mat4(glm::mat3(view)));
      def.setMat4("projection", projection);

      skyboxVao.bind();
      GLcall(glActiveTexture(GL_TEXTURE0));
      GLcall(glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex));
      glDrawArrays(GL_TRIANGLES, 0, 36);
      // glDepthMask(GL_TRUE);
      glDepthFunc(GL_LESS);

      glfwSwapBuffers(window);
    }
  }

  glfwTerminate();
  return 0;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
  SCR_WIDTH = width;
  SCR_HEIGHT = height;
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  camera.process_movement(window, speed, deltaTime);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
  camera.mouse_callback(window, xpos, ypos, &firstMouse, sensitivity);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  camera.scroll_callback(window, xoffset, yoffset);
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
  unsigned int textureID;
  GLcall(glGenTextures(1, &textureID));
  GLcall(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++)
  {
    unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
      GLcall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                          0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
      stbi_image_free(data);
    }
    else
    {
      std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }
  GLcall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GLcall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  GLcall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  GLcall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  GLcall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

  return textureID;
}