#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>

#include <stdexcept>

#include <camera.h>
#include <entity.h>

unsigned int scr_width = 1280, scr_height = 720;
Camera camera(glm::vec3(0.0f), 45.0f, 0.1f, 25.0f);
bool camera_movement = false;

// timing
float delta_time = 0.0f, last_frame = 0.0f;

void framebufferSizeCallback(GLFWwindow *, int width, int height);
void processInput(GLFWwindow *window);
void cursorPosCallback(GLFWwindow *window, double xposin, double yposin);
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(scr_width, scr_height, "scene graph", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    throw std::runtime_error("Failed to create a glfw window");
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGL(glfwGetProcAddress))
    throw std::runtime_error("Failed to initialize opengl function pointers");

  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glfwSetMouseButtonCallback(window, mouseButtonCallback);
  glfwSetScrollCallback(window, scrollCallback);
  glEnable(GL_DEPTH_TEST);

  {
    Shader ourShader("./shaders/shader.vert", "./shaders/shader.frag");

    const char *pathStr = "../resources/objects/planet/planet.obj";
    Entity ourEntity(pathStr);
    const float scale = 0.75f;
    ourEntity.transform.setLocalPosition(glm::vec3(10.0f, 0.0f, 0.0f));
    ourEntity.transform.setLocalScale(glm::vec3(scale));

    {
      Entity *lastEntity = &ourEntity;

      for (unsigned int i = 0; i < 10; ++i) {
        lastEntity->addChild(pathStr);
        lastEntity = lastEntity->children.back().get();

        // Set tranform values
        lastEntity->transform.setLocalPosition(glm::vec3(10.0f, 0.0f, 0.0f));
        lastEntity->transform.setLocalScale(glm::vec3(scale));
      }
    }
    ourEntity.updateSelfAndChild();

    glClearColor(0.4, 0.4, 0.4, 0.4);
    while (!glfwWindowShouldClose(window)) {
      float curr_frame = glfwGetTime();
      delta_time = curr_frame - last_frame;
      last_frame = curr_frame;

      camera.updateFrame();
      processInput(window);

      glfwPollEvents();
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

      // std::cout << "check" << std::endl;
      ourShader.bind();
      glm::mat4 projection = glm::perspective(
          camera.getFov(), (float)scr_width / (float)scr_height, 0.1f,
          10000.0f);
      ourShader.setMat4("view", camera.getViewMatrix());
      ourShader.setMat4("projection", projection);

      Entity *lastEntity = &ourEntity;
      while (lastEntity->children.size()) {
        ourShader.setMat4("model", lastEntity->transform.getModelMatrix());
        lastEntity->draw(ourShader);
        lastEntity = lastEntity->children.back().get();
      }

      glm::vec3 rotation = ourEntity.transform.getLocalRotation() +
                           glm::vec3(0.0f, 20 * delta_time, 0.0f);
      ourEntity.transform.setLocalRotation(rotation);
      ourEntity.updateSelfAndChild();

      glfwSwapBuffers(window);
    }
  }

  glfwTerminate();
}

void framebufferSizeCallback(GLFWwindow *, int width, int height) {
  scr_width = width;
  scr_height = height;
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  camera.processMovement(window);
}

void cursorPosCallback(GLFWwindow *window, double xposin, double yposin) {
  float xpos = static_cast<float>(xposin);
  float ypos = static_cast<float>(yposin);
  camera.updateView(xpos, ypos);
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
  // ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  // if (ImGui::GetIO().WantCaptureMouse)
  //   return;

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    camera_movement = true;
  } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    camera.firstMouse = true;
    camera_movement = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, nullptr);
  }
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  float yoff = static_cast<float>(yoffset);
  camera.updateZoom(yoff);
}
