#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
  Camera();
  Camera(glm::vec3 pos, float fov, GLFWwindow *window, unsigned int width, unsigned int height);
  float get_fov();
  void mouse_callback(GLFWwindow *window, double xposIn, double yposIn, bool *firstMouse, float sensitivity);
  void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
  void process_movement(GLFWwindow *window, float speed, float deltaTime);
  glm::mat4 get_view_matrix();

private:
  float fov = 45.0f, yaw = -90.0f, pitch = 0.0f;
  float lastX, lastY;
  glm::vec3 pos;
  glm::vec3 front;
  glm::vec3 up;
};

#endif