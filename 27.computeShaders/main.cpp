#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/imgui.h>

#include <iostream>
#include <sys/types.h>
#include <vector>

#include <camera.h>
#include <indexBuffer.h>
#include <shader.h>
#include <vertexArray.h>
#include <vertexBuffer.h>
#include <vertexBufferLayout.h>

unsigned int scr_width = 1280, scr_height = 720;
bool camera_movement = false, wireframe = true;
Camera camera(glm::vec3(0.0f), 45.0f, 0.1f, 25.5f);

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void cursorPosCallback(GLFWwindow *window, double xposin, double yposin);
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window =
      glfwCreateWindow(scr_width, scr_height, "TITLE_HERE", NULL, NULL);
  if (window == nullptr) {
    std::cout << "Failed to create a GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize opengl function pointers" << std::endl;
    return -1;
  }
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glfwSetMouseButtonCallback(window, mouseButtonCallback);
  glfwSetScrollCallback(window, scrollCallback);
  glEnable(GL_DEPTH_TEST);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

  ImGui_ImplGlfw_InitForOpenGL(window, false);
  ImGui_ImplOpenGL3_Init("#version 430 core");

  {
    Shader shader("../shaders/vertex.glsl", "../shaders/fragment.glsl");
    ComputeShader compute("../shaders/compute.glsl");

    const unsigned int TEXTURE_WIDTH = 512, TEXTURE_HEIGHT = 512;
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE); // set texture wrapping to GL_REPEAT
                                       // (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0,
                 GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // v.x
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f, // v.y
        -1.0f, 1.0f,  0.0f, 0.0f, 1.0f, // v.z

        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // v.x
        1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, // v.y
        1.0f,  1.0f,  0.0f, 1.0f, 1.0f  // v.z
    };

    VertexArray vao;
    VertexBuffer vbo(sizeof(vertices), vertices, GL_STATIC_DRAW);
    VertexBufferLayout layout;
    // xyz
    layout.push<float>(3);
    layout.push<float>(2);
    vao.addBuffer(vbo, layout);

    glClearColor(0.4, 0.4, 0.4, 0.4);
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    while (!glfwWindowShouldClose(window)) {
      glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
      camera.updateFrame();

      glfwPollEvents();
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      processInput(window);

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      {
        ImGui::Begin("params");
        ImGui::Checkbox("wireframe", &wireframe);
        ImGui::End();
      }

      compute.bind();
      glDispatchCompute((unsigned int)TEXTURE_WIDTH,
                        (unsigned int)TEXTURE_HEIGHT, 1);
      glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

      shader.bind();
      shader.setInt("tex", 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);

      vao.bind();
      glDrawArrays(GL_TRIANGLES, 0, 6);

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

      glfwSwapBuffers(window);
    }
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
  return 0;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
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
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  if (ImGui::GetIO().WantCaptureMouse)
    return;

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
