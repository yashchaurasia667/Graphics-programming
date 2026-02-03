#include "shader.h"

Shader::Shader(const char *vertexPath, const char *fragmentPath)
{
  std::string vertexSource = getShaderSource(vertexPath);
  std::string fragmentSource = getShaderSource(fragmentPath);
  const char *vShaderCode = vertexSource.c_str();
  const char *fShaderCode = fragmentSource.c_str();

  // COMPILE SHADERS
  unsigned int vertex, fragment;

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

void Shader::addGeometryShader(const char *geometryPath)
{
  std::string geometrySource = getShaderSource(geometryPath);
  const char *gShaderCode = geometrySource.c_str();
  unsigned int geometry;

  geometry = glCreateShader(GL_GEOMETRY_SHADER);
  glShaderSource(geometry, 1, &gShaderCode, NULL);
  glCompileShader(geometry);
  checkCompileErrors(geometry, "GEOMETRY");

  glAttachShader(ID, geometry);
  glLinkProgram(ID);
  checkCompileErrors(ID, "PROGRAM");

  glDeleteShader(geometry);
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
void Shader::setMat4(const std::string &name, glm::mat4 value) const
{
  glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setVec3(const std::string &name, glm::vec3 value) const
{
  glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setVec2(const std::string &name, glm::vec2 value) const
{
  glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
}

std::string Shader::getShaderSource(const char *path)
{
  std::ifstream shader_file(path);
  if (!shader_file)
  {
    std::cout << "ERROR::SHADER::" << path << "::NOT SUCCESSFULLY READ" << std::endl;
    return "";
  }

  std::cout << "Successfully read shader: " << path << std::endl;
  std::stringstream buffer;
  buffer << shader_file.rdbuf();
  std::string source = buffer.str();
  std::cout << "Shader source length: " << source.length() << std::endl;
  return source;
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