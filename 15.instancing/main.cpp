#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <utils.h>
#include <shader.h>
#include <camera.h>
#include <vertexArray.h>
#include <vertexBuffer.h>
#include <vertexBufferLayout.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
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
  {
    glm::vec2 translations[100];
    int index = 0;
    float offset = 0.1f;
    for (int y = -10; y < 10; y += 2)
      for (int x = -10; x < 10; x += 2)
      {
        glm::vec2 translation;
        translation.x = (float)x / 10.0f + offset;
        translation.y = (float)y / 10.0f + offset;
        translations[index++] = translation;
      }

    Shader shader("../shaders/instancing.vs", "../shaders/instancing.fs");

    float quadVertices[] = {
        // positions   // colors
        -0.05f, 0.05f, 1.0f, 0.0f, 0.0f,
        0.05f, -0.05f, 0.0f, 1.0f, 0.0f,
        -0.05f, -0.05f, 0.0f, 0.0f, 1.0f,
        -0.05f, 0.05f, 1.0f, 0.0f, 0.0f,
        0.05f, -0.05f, 0.0f, 1.0f, 0.0f,
        0.05f, 0.05f, 0.0f, 1.0f, 1.0f};
    VertexArray vao;
    VertexBuffer vbo(quadVertices, 6 * 5, GL_STATIC_DRAW);
    VertexBufferLayout layout;
    layout.push<float>(2);
    layout.push<float>(3);
    vao.addBuffer(vbo, layout);

    unsigned int instanceVbo;
    glGenBuffers(1, &instanceVbo);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      shader.use();
      shader.setFloat("time", glfwGetTime());
      vao.bind();
      glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);

      glfwPollEvents();
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
