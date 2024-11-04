#include "utils.h"
#include "triangle.h"

// creating shader
unsigned int make_module(const std::string &filepath, unsigned int module_type)
{
  std::ifstream file;
  std::stringstream bufferedLines;
  std::string line;

  file.open(filepath);
  while (std::getline(file, line))
  {
    bufferedLines << line << "\n";
  }

  std::string source = bufferedLines.str();
  const char *shaderSrc = source.c_str();
  bufferedLines.str("");
  file.close();

  unsigned int shaderModule = glCreateShader(module_type);
  glShaderSource(shaderModule, 1, &shaderSrc, NULL);
  glCompileShader(shaderModule);

  int success;
  glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);

  if (!success)
  {
    char errorLog[1024];
    glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
    std::cout << "Failed to compile shaders \n"
              << errorLog << std::endl;
  }
  return shaderModule;
}

// using shader
unsigned int make_shader(const char *vertex_filepath, const char *fragment_filepath)
{
  std::vector<unsigned int> modules;
  modules.push_back(make_module(vertex_filepath, GL_VERTEX_SHADER));
  modules.push_back(make_module(fragment_filepath, GL_FRAGMENT_SHADER));

  unsigned int shader = glCreateProgram();
  for (unsigned int shaderModule : modules)
  {
    glAttachShader(shader, shaderModule);
  }

  glLinkProgram(shader);

  int success;
  glGetProgramiv(shader, GL_LINK_STATUS, &success);

  if (!success)
  {
    char errorLog[1024];
    glGetProgramInfoLog(shader, 1024, NULL, errorLog);
    std::cout << "Failed to link shaders\n"
              << errorLog << std::endl;
  }

  for (unsigned int shaderModule : modules)
  {
    glDeleteShader(shaderModule);
  }

  return shader;
}

int main()
{
  int width = 800, height = 600;
  GLFWwindow *window;

  glfwSetErrorCallback(errorCallback);

  if (initGLFW(&window) != 0)
  {
    std::cout << "Something went wrong" << std::endl;
    return -1;
  }

  if (initGlad(&window) != 0)
  {
    std::cout << "Couldn't initialize glad\n"
              << std::endl;
    return -1;
  }

  glClearColor(0.0235f, 0.2784f, 0.1098f, 1.0f);

  unsigned int shader = make_shader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");

  Triangle *triangle = new Triangle();

  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader);
    triangle->draw();
    glfwSwapBuffers(window);
  }

  triangle->~Triangle();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
