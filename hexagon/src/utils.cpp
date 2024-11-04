#include "utils.h"

#define WINW 800
#define WINH 600

void errorCallback(int error, const char *description)
{
  fprintf(stderr, "GLFW error [%d]: %s \n", error, description);
}

void frameBufferCallback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
  printf("width: %d\n", width);
}

int initGLFW(GLFWwindow **window)
{
  // int width = 800, height = 600;

  if (!glfwInit())
  {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  *window = glfwCreateWindow(WINW, WINH, "Hexagon", NULL, NULL);

  if (!*window)
  {
    fprintf(stderr, "Failed to create a GLFW window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(*window);
  glfwSetFramebufferSizeCallback(*window, frameBufferCallback);
  return 0;
}

int initGlad(GLFWwindow **window)
{
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    fprintf(stderr, "Failed to laod OpenGL function pointers\n");
    glfwDestroyWindow(*window);
    glfwTerminate();
    return -1;
  }

  glViewport(0, 0, WINW, WINH);
  return 0;
}
