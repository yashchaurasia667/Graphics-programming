#include <camera.h>
#include <iostream>

Camera::Camera()
{
  this->fov = 45.0f;
  this->pos = glm::vec3(0.0f);
  this->front = glm::vec3(0.0f, 0.0f, -1.0f);
  this->up = glm::vec3(0.0f, 1.0f, 0.0f);
  this->lastX = 0;
  this->lastY = 0;
}

Camera::Camera(glm::vec3 pos, float fov, GLFWwindow *window, unsigned int width, unsigned int height)
{
  this->fov = fov;
  this->pos = pos;
  this->front = glm::vec3(0.0f, 0.0f, -1.0f);
  this->up = glm::vec3(0.0f, 1.0f, 0.0f);
  this->lastX = width / 2.0f;
  this->lastY = height / 2.0f;
}

void Camera::mouse_callback(GLFWwindow *window, double xposIn, double yposIn, bool *firstMouse, float sensitivity)
{
  // std::cout << "mouse pos" << std::endl;
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (*firstMouse)
  {
    std::cout << "first mouse" << std::endl;
    lastX = xpos;
    lastY = ypos;
    *firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;

  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  this->front = glm::normalize(front);
}

void Camera::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  // std::cout << "scroll" << std::endl;
  fov -= (float)yoffset;
  if (fov < 1.0f)
    fov = 1.0f;
  if (fov > 45.0f)
    fov = 45.0f;
}

float Camera::get_fov()
{
  return fov;
}

void Camera::process_movement(GLFWwindow *window, float speed, float deltaTime)
{
  float cameraSpeed = speed * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  {
    pos += cameraSpeed * front;
    // std::cout << "keyboard" << std::endl;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    pos -= cameraSpeed * front;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    pos -= glm::normalize(glm::cross(front, up)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    pos += glm::normalize(glm::cross(front, up)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    pos += up * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    pos -= up * cameraSpeed;
}

glm::mat4 Camera::get_view_matrix()
{
  return glm::lookAt(pos, pos + front, up);
}