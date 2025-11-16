## General build guide

<h3>Dependencies</h3>

- [CMake](https://cmake.org/download/)
- [Make](https://www.gnu.org/software/make/#download)
- A C/C++ compiler, I use [MinGW](technology@curepoint.in)

### For Linux

```bash
$ cd {folderName}
$ cmake . -B build
$ cd build && make
```

### For Windows

```powershell
$ cd {folderName}
$ cmake . -B build "MinGW Makefiles"
$ cd build && mingw32-make
```

## Basic Structures of several programs

### Table of contents

- [Creating a window](#creating-a-window)
- [Basic Vertex Shader](#vertex-shader)
- [Basic Fragment Shader](#fragment-shader)
- [Shader Class](#shader-class)
- [Triangle](#triangle)

### Creating a window

```cpp
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

const unsigned int SCR_WIDTH = 800, SCR_HEIGHT = 600;
int main() {
  // initialize glfw
  glfwInit();
  // we need to tell glfw which openGl version we are using
  // since we're using version 3.3 so we set both major and minor versions to be 3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  // we are using the openGl core profile as we only want the modern opengl functions and not the depricated ones
  // available options -> core/compatibility
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // create an openGL window object
  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT);
  if(window == NULL) {
    std::cout << "Failed to create an OpenGL window" << std::endl;
    glfwTerminate();
    return -1;
  }
  // tell opengl to use our newly created window
  glfwMakeContextCurrent(window);

  // Load all openGl pointers with glad
  if(!gladLoadGLLoader((GLADLoadProc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // main loop to prevent the window from closing instantly
  while(!glfwWindowShouldClose(window)) {
    // set the color which you want the window to be cleared with
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    // clear screen with the previously set color
    glClear(GL_COLOR_BUFFER_BIT);
    // display the new frame every iteration of the while loop
    glfwSwapBuffers(window);
    // poll all window events, without this the window won't respond
    glfwPollEvents();
  }

  // close the window when the while loop exits
  glfwTerminate();
  return 0;
}
```

### Vertex Shader

```glsl
#version 330 core

// inputs to the shader through attributes
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aColor;

// outputs that can be used in another shaders just need the name to be same
out vec3 ourColor;

void main() {
  gl_Position = vec4(aPos, 1.0f);
  ourColor = aColor;
}
```

### Fragment shader

```glsl
#version 330 core

// input from the vertex shader above
in vec3 ourColor;

out vec4 FragColor;

void main() {
  // rgba format
  FragColor = vec4(ourColor, 1.0f);
}
```

### Shader Class

#### shader.h

```cpp
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
  unsigned int ID;
  Shader(const char *vertexPath, const char *fragPath);
  // use the shader
  void use();
  // utility uniform functions
  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;

private:
  void checkCompileErrors(unsigned int shader, std::string type);
};

#endif
```

#### shader.cpp

```cpp
#include "shader.h"

Shader::Shader(const char *vertexPath, const char *fragmentPath)
{
  std::string vertexCode;
  std::string fragmentCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;

  vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try
  {
    // open shader files
    vShaderFile.open(vertexPath);
    fShaderFile.open(fragmentPath);
    std::stringstream vShaderStream, fShaderStream;
    // read file's buffer contents into stream
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    // close shader files
    vShaderFile.close();
    fShaderFile.close();
    // convert stream into c string
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();
  }
  catch (std::ifstream::failure e)
  {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
  }
  const char *vShaderCode = vertexCode.c_str();
  const char *fShaderCode = fragmentCode.c_str();

  // COMPILE SHADERS
  unsigned int vertex, fragment;
  int success;
  char infoLog[512];

  // vertex shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vShaderCode, NULL);
  glCompileShader(vertex);
  checkCompileErrors(vertex, "VERTEX");

  // fragment shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fShaderCode, NULL);
  glCompileShader(fragment);
  checkCompileErrors(fragment, "FRAGMENT");

  // shader program
  ID = glCreateProgram();
  glAttachShader(ID, vertex);
  glAttachShader(ID, fragment);
  glLinkProgram(ID);
  checkCompileErrors(ID, "PROGRAM");

  glDeleteShader(vertex);
  glDeleteShader(fragment);
}

void Shader::use()
{
  glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const
{
  glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setInt(const std::string &name, int value) const
{
  glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string &name, float value) const
{
  glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
  int success;
  char infoLog[512];
  if (type != "PROGRAM")
  {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(shader, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
  }
  else
  {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success)
    {
      glGetProgramInfoLog(shader, 512, NULL, infoLog);
      if (!success)
      {
        glGetProgramInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
      }
    }
  }
}
```

### Triangle

```cpp
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

const unsigned int SCR_WIDTH=800, SCR_HEIGHT=600;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

int main() {
  /* -----------------------------------------------------------
  Create a window heree
  --------------------------------------------------------------
  */
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // load all openGL function pointers
  if(!gladLoadGLLoader((GladLoadProc)glfwGetProcAddress)) {
    std::cout << "Failed to load OpenGL drivers" <<  std::endl;
    return -1;
  }
  
  // create a shader object
  Shader ourShader("path/to/vertex/shader", "path/to/fragment/shader");

  // setup vertex data and attributes here
  float vertices[] = {
    // positions (x, y, z)   // colors (r, g, b)
    -0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.0f,      0.0f, 1.0f, 0.0f,
    0.0f, 0.5f, 0.0f,       0.0f, 0.0f, 1.0f
  };
  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO):
  glGenBuffers(1, &VBO);
  glBindVertexArrays(VAO);

  glBindBuffers(GL_ARRAY_BUFFER, VAO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  while(!glfwWindowShouldClose(window)) {
    processInput();
    glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ourShader.use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glfwTerminate();
  return 0;
}

// UTILITY FUNCTIONS FOR QUALITY OF LIFE
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}
```
