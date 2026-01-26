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

unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;

bool firstMouse = true;
float sensitivity = 0.1f, speed = 2.5f;
float deltaTime = 0.0f, lastFrame = 0.0f;
Camera camera;

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

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
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

  float cubeVertices[] = {
      // positions          // texture Coords
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

      -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
      -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
      -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
      -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

  float planeVertices[] = {
      // positions          // texture Coords
      // (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode).
      // this will cause the floor texture to repeat)
      5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
      -5.0f, -0.5f, 5.0f, 0.0f, 0.0f,
      -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

      5.0f, -0.5f, 5.0f, 2.0f, 0.0f,
      -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,
      5.0f, -0.5f, -5.0f, 2.0f, 2.0f};

  float quadVertices[] = {
      // positions   // texCoords
      -1.0f, 1.0f, 0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f, 0.0f,
      1.0f, -1.0f, 1.0f, 0.0f,

      -1.0f, 1.0f, 0.0f, 1.0f,
      1.0f, -1.0f, 1.0f, 0.0f,
      1.0f, 1.0f, 1.0f, 1.0f};

  {
    camera = Camera(glm::vec3(0.0f), 45.0f, window, SCR_WIDTH, SCR_HEIGHT);
    Shader def("../shaders/default.vs", "../shaders/default.fs");
    Shader fbs("../shaders/framebuffer.vs", "../shaders/framebuffer.fs");

    VertexArray cubeVao;
    VertexBuffer cubeVbo(cubeVertices, 5 * 36, GL_STATIC_DRAW);
    VertexBufferLayout layout;

    layout.push<float>(3);
    layout.push<float>(2);
    cubeVao.addBuffer(cubeVbo, layout);

    VertexArray planeVao;
    VertexBuffer planeVbo(planeVertices, 5 * 6, GL_STATIC_DRAW);
    planeVao.addBuffer(planeVbo, layout);

    VertexArray quadVao;
    VertexBuffer quadVbo(quadVertices, 4 * 6, GL_STATIC_DRAW);
    VertexBufferLayout quadLayout;
    quadLayout.push<float>(2);
    quadLayout.push<float>(2);
    quadVao.addBuffer(quadVbo, quadLayout);

    Texture cubeTex("../../assets/container.jpg");
    Texture planeTex("../../assets/metal.png");

    def.use();
    def.setInt("u_texture", 0);

    fbs.use();
    fbs.setInt("screenTexture", 0);

    // ----------------- FRAMEBUFFER STUFFF -------------------------
    unsigned int framebuffer;
    GLcall(glGenFramebuffers(1, &framebuffer));
    GLcall(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));

    unsigned int texColorBuffer;
    GLcall(glGenTextures(1, &texColorBuffer));
    GLcall(glBindTexture(GL_TEXTURE_2D, texColorBuffer));
    GLcall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr));
    GLcall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLcall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLcall(glBindTexture(GL_TEXTURE_2D, 0));

    GLcall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0));

    unsigned int rbo;
    GLcall(glGenRenderbuffers(1, &rbo));
    GLcall(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
    GLcall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT));
    GLcall(glBindRenderbuffer(GL_RENDERBUFFER, 0));

    GLcall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    GLcall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    // ----------------------------------------------------------------

    // draw as wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (!glfwWindowShouldClose(window))
    {
      glfwPollEvents();

      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;
      processInput(window);

      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
      glEnable(GL_DEPTH_TEST);

      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      int width = 0, height = 0;
      glfwGetFramebufferSize(window, &width, &height);

      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 projection = glm::perspective(camera.get_fov(), (float)width / (float)height, 0.1f, 100.0f);

      def.use();
      def.setMat4("view", camera.get_view_matrix());
      def.setMat4("projection", projection);

      cubeVao.bind();
      cubeTex.bind();
      model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
      def.setMat4("model", model);
      GLcall(glDrawArrays(GL_TRIANGLES, 0, 36));

      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
      def.setMat4("model", model);
      GLcall(glDrawArrays(GL_TRIANGLES, 0, 36));

      planeVao.bind();
      planeTex.bind();
      def.setMat4("model", glm::mat4(1.0f));
      GLcall(glDrawArrays(GL_TRIANGLES, 0, 36));

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glDisable(GL_DEPTH_TEST);

      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      fbs.use();
      quadVao.bind();
      glBindTexture(GL_TEXTURE_2D, texColorBuffer);
      glDrawArrays(GL_TRIANGLES, 0, 6);

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