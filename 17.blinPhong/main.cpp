#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <shader.h>
#include <camera.h>

unsigned int scr_width = 1280, scr_height = 720;
Camera camera;

float deltaTime = 0.0f, lastFrame = 0.0f;
bool fMouse = true;

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double x, double y);

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(scr_width, scr_height, "BLIN PHONG LIGHTING", NULL, NULL);
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
  glEnable(GL_DEPTH_TEST);

  {
    camera = Camera(glm::vec3(0.0f, 0.0f, 1.0), 45.0f, window, scr_width, scr_height);
    Shader phong("../shaders/plane.vs", "../shaders/phong.fs");
    Shader bphong("../shaders/plane.vs", "../shaders/blin_phong.fs");

    float vertices[] = {
        -0.5, -0.5, -0.5, 0.0f, 1.0f, 0.0f,
        0.5, -0.5, -0.5, 0.0f, 1.0f, 0.0f,
        0.5, -0.5, 0.5, 0.0f, 1.0f, 0.0f,

        -0.5, -0.5, -0.5, 0.0f, 1.0f, 0.0f,
        0.5, -0.5, 0.5, 0.0f, 1.0f, 0.0f,
        -0.5, -0.5, 0.5, 0.0f, 1.0f, 0.0f};

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

    while (!glfwWindowShouldClose(window))
    {
      float currFrame = glfwGetTime();
      deltaTime = currFrame - lastFrame;
      lastFrame = currFrame;

      glfwPollEvents();
      glClearColor(0.5f, 0.3f, 0.2f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      processInput(window);

      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
      glm::mat4 projection = glm::perspective(camera.get_fov(), (float)scr_width / (float)scr_height, 0.1f, 100.0f);

      phong.use();
      phong.setMat4("model", model);
      phong.setMat4("view", camera.get_view_matrix());
      phong.setMat4("projection", projection);
      phong.setVec3("lightPos", glm::vec3(-1.2f, 0.0f, 0.5f));
      phong.setVec3("viewPos", camera.get_pos());

      glDrawArrays(GL_TRIANGLES, 0, 6);

      model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
      bphong.use();
      bphong.setMat4("model", model);
      bphong.setMat4("view", camera.get_view_matrix());
      bphong.setMat4("projection", projection);
      bphong.setVec3("lightPos", glm::vec3(1.2f, 0.0f, 0.5f));
      bphong.setVec3("viewPos", camera.get_pos());
      glDrawArrays(GL_TRIANGLES, 0, 6);

      glfwSwapBuffers(window);
    }
  }
  glfwTerminate();
  return 0;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
  scr_width = width;
  scr_height = height;
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  camera.process_movement(window, 2.5f, deltaTime);
}

void mouse_callback(GLFWwindow *window, double x, double y) {
  camera.mouse_callback(window, x, y, &fMouse, 0.1f);
}