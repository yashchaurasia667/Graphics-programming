#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{
public:
  glm::vec3 cameraPos;
  glm::vec3 cameraFront;
  glm::vec3 cameraUp;

  Camera(glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp, GLFWwindow *window);
  void processMovement(GLFWwindow *window, float speed, float deltaTime);
  glm::mat4 getViewMatrix();
  float getFov();

private:
  bool firstMouse = true;
  float yaw = -90.0f;
  float pitch = 0.0f;
  float lastX = 800.0f / 2.0f;
  float lastY = 600.0f / 2.0f;
  float fov = 45.0f;

  static void zoomOnScroll(GLFWwindow *window, double xoffset, double yoffset);
  static void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
  void onMouseMove(double xpos, double ypos);
  void zoomLogic(double xoffset, double yoffset);
};

#endif