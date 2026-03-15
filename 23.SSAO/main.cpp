#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <shader.h>
#include <camera.h>

#define STB_IMAGE_IMPLEMENTATION
#include <utils.h>

unsigned int scr_width = 1280, scr_height = 720;
float delta_time, last_frame = 0.0f;
bool first_mouse = true;

Camera camera;

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void cursorPosCallback(GLFWwindow *window, double xposin, double yposin);
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window = glfwCreateWindow(scr_width, scr_height, "TITLE_HERE", NULL, NULL);
  if (window == nullptr)
  {
    std::cout << "Failed to create a GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize opengl function pointers" << std::endl;
    return -1;
  }
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetScrollCallback(window, scrollCallback);
  glEnable(GL_DEPTH_TEST);

  {
    camera = Camera(glm::vec3(0.0f), 45.0f, window, scr_width, scr_height);
    Shader geometryPass("../shaders/g_buffer.vs", "../shaders/g_buffer.fs");
    Shader lightingPass("../shaders/deferred_shading.vs", "../shaders/deferred_shading.fs");

    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    unsigned int gPosition;
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, scr_width, scr_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for (unsigned int i = 0; i < 64; ++i)
    {
      glm::vec3 sample(
          randomFloats(generator) * 2.0 - 1.0,
          randomFloats(generator) * 2.0 - 1.0,
          randomFloats(generator));
      sample = glm::normalize(sample);
      sample *= randomFloats(generator);
      ssaoKernel.push_back(sample);
    }

    glClearColor(0.4, 0.4, 0.4, 0.4);
    while (!glfwWindowShouldClose(window))
    {
      float curr_frame = glfwGetTime();
      delta_time = curr_frame - last_frame;
      last_frame = curr_frame;

      glfwPollEvents();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      processInput(window);

      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 view = camera.get_view_matrix();
      glm::mat4 projection = glm::perspective(camera.get_fov(), (float)scr_width / (float)scr_height, 0.1f, 100.0f);

      geometryPass.use();
      geometryPass.setMat4("model", model);
      geometryPass.setMat4("view", view);
      geometryPass.setMat4("projection", projection);

      renderCube();

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
  camera.process_movement(window, 2.5f, delta_time);
}

void cursorPosCallback(GLFWwindow *window, double xposin, double yposin)
{
  camera.mouse_callback(window, xposin, yposin, &first_mouse, 0.1f);
}
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
  camera.scroll_callback(window, xoffset, yoffset);
}