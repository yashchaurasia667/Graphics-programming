#include "utils.h"
#include "triangle.h"

using namespace std;

// creating shader
unsigned int make_module(const string &filepath, unsigned int module_type)
{
  ifstream file;
  stringstream bufferedLines;
  string line;

  file.open(filepath);
  while (getline(file, line))
  {
    bufferedLines << line << "\n";
  }

  string source = bufferedLines.str();
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
    cout << "Failed to compile shaders \n"
         << errorLog << endl;
  }
  return shaderModule;
}

// using shader
unsigned int make_shader(const char *vertex_filepath, const char *fragment_filepath)
{
  vector<unsigned int> modules;
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
    cout << "Failed to link shaders\n"
         << errorLog << endl;
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
    cout << "Something went wrong" << endl;
    return -1;
  }

  if (initGlad(&window) != 0)
  {
    cout << "Couldn't initialize glad\n"
         << endl;
    return -1;
  }

  glClearColor(0.9686f, 0.8431f, 0.0196f, 1.0f);

  unsigned int shader = make_shader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");

  vector<float> positions = {
      0.0f, 0.0f, 0.0f,   // screen middle
      0.0f, 0.75f, 0.0f,   // mid top
      -0.5f, 0.30f, 0.0f,  // left top mid x, mid y
      0.5f, 0.30f, 0.0f,   // right top mid x, mid y
      0.5f, -0.30f, 0.0f,  // right bottom mid x, mid y
      0.0f, -0.75f, 0.0f,  // mid bottom
      -0.5f, -0.30f, 0.0f, // left bottom mid x, mid y
  };

  vector<int> colors = {3, 3, 3, 3, 3, 3, 3};
  vector<int> elements = {
      0, 1, 2,
      0, 1, 3,
      0, 4, 3,
      0, 4, 5, 
      0, 6, 5, 
      0, 6, 2
      };

  Triangle *triangle = new Triangle(positions, colors, elements.size(), elements);

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
