#include "config.h"
#include "triangle_mesh.h"
#include "material.h"

void frameBufferSizeCallback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, true);
  }
}

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

  std::string shaderSource = bufferedLines.str();
  const char *shaderSrc = shaderSource.c_str();
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
    std::cout << "Shader Module compilation error: \n"
              << errorLog << std::endl;
  }

  return shaderModule;
}

unsigned int make_shader(const std::string &vertex_filepath, const std::string &fragment_filepath)
{
  std::vector<unsigned int> modules;

  // Add a vertex shader module
  modules.push_back(make_module(vertex_filepath, GL_VERTEX_SHADER));

  // Add a fragment shader module
  modules.push_back(make_module(fragment_filepath, GL_FRAGMENT_SHADER));

  // Attach all the modules then link the program
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
    std::cout << "Shader Linking error: \n"
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

  // Initialize GLFW
  if (!glfwInit())
  {
    std::cout << "GLFW couldn't start" << std::endl;
    return -1;
  }

  // Setting GLFW version to 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // Using GLFW core profile (CORE profile for modern functions and compatible for older functions)
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

  // creating a GLFW window
  GLFWwindow *window = glfwCreateWindow(800, 600, "hello world", nullptr, nullptr);

  if (!window)
  {
    std::cout << "GLFW couldn't create a window" << std::endl;
    return -1;
  }

  // Introduce the window into the current context
  glfwMakeContextCurrent(window);

  // Callback for when the window is resized
  glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

  // Initialize OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to load OpenGL function pointers" << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    return -1;
  }

  // Specify the color of the background
  glClearColor(0.25f, 0.5f, 0.75f, 1.0f);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  // left, top, width, height
  glViewport(0, 0, width, height);

  TriangleMesh *triangle = new TriangleMesh();
  Material *material = new Material("../img/mouse.jpeg");

  unsigned int shader = make_shader(
      "../src/shaders/vertex.txt",
      "../src/shaders/fragment.txt");

  // Run while window should not close
  while (!glfwWindowShouldClose(window))
  {
    processInput(window);
    glfwPollEvents();

    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader);
    material->use();
    triangle->draw();

    // Swap front and back frames
    glfwSwapBuffers(window);
  }

  glDeleteProgram(shader);
  delete triangle;
  glfwTerminate();

  return 0;
}