#include <camera.h>

Camera::Camera(glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp, GLFWwindow *window)
{
  Camera::cameraPos = cameraPos;
  Camera::cameraFront = cameraFront;
  Camera::cameraUp = cameraUp;

  glfwSetWindowUserPointer(window, this);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, Camera::mouse_callback);
  glfwSetScrollCallback(window, Camera::zoomOnScroll);
}

void Camera::processMovement(GLFWwindow *window, float speed, float deltaTime)
{
  float cameraSpeed = speed * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}

glm::mat4 Camera::getViewMatrix()
{
  return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

float Camera::getFov()
{
  return fov;
}

void Camera::zoomOnScroll(GLFWwindow *window, double xoffset, double yoffset)
{
  Camera *cam = static_cast<Camera *>(glfwGetWindowUserPointer(window));
  cam->zoomLogic(xoffset, yoffset);
}

void Camera::zoomLogic(double xoffset, double yoffset)
{
  fov -= (float)yoffset;
  if (fov < 1.0f)
    fov = 1.0f;
  if (fov > 45.0f)
    fov = 45.0f;
}

void Camera::mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
  Camera *cam = static_cast<Camera *>(glfwGetWindowUserPointer(window));
  cam->onMouseMove(xposIn, yposIn);
}

void Camera::onMouseMove(double xposIn, double yposIn)
{
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  if (firstMouse)
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;
  float sensitivity = 0.1f;

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
  cameraFront = glm::normalize(front);
}