#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <iostream>
#include <vector>

#include <shader.h>
#include <camera.h>
#include <vertexArray.h>
#include <vertexBuffer.h>
#include <indexBuffer.h>
#include <vertexBufferLayout.h>

#define STBI_IMAGE_IMPLEMENTATION
#include <stb_image.h>

unsigned int scr_width = 1280, scr_height = 720;
bool camera_movement = false, wireframe = true;
Camera camera(glm::vec3(0.0f), 45.0f, 0.1f, 2.5f);

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void cursorPosCallback(GLFWwindow *window, double xposin, double yposin);
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window = glfwCreateWindow(scr_width, scr_height, "TITLE_HERE", NULL, NULL);
  if (window == nullptr)
  {
    std::cout << "Failed to create a GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
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
  ImGui_ImplOpenGL3_Init("#version 330 core");

  {
    int width, height, nrChannels;
    unsigned char *data = stbi_load("../../assets/iceland_heightmap.png", &width, &height, &nrChannels, 0);

    std::vector<float> vertices;
    float yScale = 64.0f / 256.0f, yShift = 16.0f;
    for (unsigned int i = 0; i < height; i++)
    {
      for (unsigned int j = 0; j < width; j++)
      {
        unsigned char *texel = data + (j + width * i) * nrChannels;
        unsigned char y = texel[0];

        vertices.push_back(-height / 2.0f + i);
        vertices.push_back((int)y * yScale - yShift);
        vertices.push_back(-width / 2.0f + j);
      }
    }
    stbi_image_free(data);

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < height - 1; i++)
    {
      for (unsigned int j = 0; j < width; j++)
      {
        for (unsigned int k = 0; k < 2; k++)
        {
          indices.push_back(j + width * (i + k));
        }
      }
    }
    const unsigned int NUM_STRIPS = height - 1;
    const unsigned int NUM_VERTS_PER_STRIP = width * 2;

    Shader shader("../shaders/tessellation.vs", "../shaders/tessellation.fs");
    VertexArray vao;
    VertexBuffer vbo(vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    IndexBuffer ebo((int)indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    VertexBufferLayout layout;
    layout.push<float>(3);
    vao.addBuffer(vbo, layout);

    glClearColor(0.4, 0.4, 0.4, 0.4);
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    while (!glfwWindowShouldClose(window))
    {
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

      shader.bind();
      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 view = camera.getViewMatrix();
      glm::mat4 projection = glm::perspective(camera.getFov(), (float)scr_width / (float)scr_height, 0.1f, 1000.0f);

      shader.setMat4("model", model);
      shader.setMat4("view", view);
      shader.setMat4("projection", projection);

      vao.bind();
      for (unsigned int strip = 0; strip < NUM_STRIPS; ++strip)
      {
        glDrawElements(GL_TRIANGLE_STRIP, NUM_VERTS_PER_STRIP, GL_UNSIGNED_INT, (void *)(sizeof(unsigned int) * NUM_VERTS_PER_STRIP * strip));
      }

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

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
  scr_width = width;
  scr_height = height;
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  camera.processMovement(window);
}

void cursorPosCallback(GLFWwindow *window, double xposin, double yposin)
{
  float xpos = static_cast<float>(xposin);
  float ypos = static_cast<float>(yposin);
  camera.updateView(xpos, ypos);
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
  if (ImGui::GetIO().WantCaptureMouse)
    return;

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
  {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    camera_movement = true;
  }
  else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
  {
    camera.firstMouse = true;
    camera_movement = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, nullptr);
  }
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
  float yoff = static_cast<float>(yoffset);
  camera.updateZoom(yoff);
}
