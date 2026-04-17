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
Camera camera(glm::vec3(0.0f), 45.0f, 0.1f, 25.5f);

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void cursorPosCallback(GLFWwindow* window, double xposin, double yposin);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "TITLE_HERE", NULL, NULL);
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
    Shader shader("../shaders/tessellation.vs", "../shaders/tessellation.fs", nullptr, "../shaders/control.tcs", "../shaders/evaluation.tes");

    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load("../../assets/iceland_heightmap.png", &width, &height, &nrChannels, 0);
    if (data) {
      shader.bind();
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);

      shader.setInt("heightMap", 0);
      std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    }
    else {
      std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // vertex generation
    std::vector<float> vertices;

    unsigned rez = 20;
    for (unsigned i = 0; i <= rez - 1; i++)
    {
      for (unsigned j = 0; j <= rez - 1; j++)
      {
        vertices.push_back(-width / 2.0f + width * i / (float)rez);   // v.x
        vertices.push_back(0.0f);                                     // v.y
        vertices.push_back(-height / 2.0f + height * j / (float)rez); // v.z
        vertices.push_back(i / (float)rez);                           // u
        vertices.push_back(j / (float)rez);                           // v

        vertices.push_back(-width / 2.0f + width * (i + 1) / (float)rez); // v.x
        vertices.push_back(0.0f);                                         // v.y
        vertices.push_back(-height / 2.0f + height * j / (float)rez);     // v.z
        vertices.push_back((i + 1) / (float)rez);                         // u
        vertices.push_back(j / (float)rez);                               // v

        vertices.push_back(-width / 2.0f + width * i / (float)rez);         // v.x
        vertices.push_back(0.0f);                                           // v.y
        vertices.push_back(-height / 2.0f + height * (j + 1) / (float)rez); // v.z
        vertices.push_back(i / (float)rez);                                 // u
        vertices.push_back((j + 1) / (float)rez);                           // v

        vertices.push_back(-width / 2.0f + width * (i + 1) / (float)rez);   // v.x
        vertices.push_back(0.0f);                                           // v.y
        vertices.push_back(-height / 2.0f + height * (j + 1) / (float)rez); // v.z
        vertices.push_back((i + 1) / (float)rez);                           // u
        vertices.push_back((j + 1) / (float)rez);                           // v
      }
    }
    const unsigned int NUM_STRIPS = height - 1;
    const unsigned int NUM_VERTS_PER_STRIP = width * 2;

    VertexArray vao;
    VertexBuffer vbo(vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
    VertexBufferLayout layout;
    // xyz, uv
    layout.push<float>(3);
    layout.push<float>(2);
    vao.addBuffer(vbo, layout);

    glPatchParameteri(GL_PATCH_VERTICES, 4);
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
      glm::mat4 projection = glm::perspective(camera.getFov(), (float)scr_width / (float)scr_height, 0.1f, 100000.0f);

      shader.setMat4("model", model);
      shader.setMat4("view", view);
      shader.setMat4("projection", projection);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
      shader.setInt("heightMap", 0);  // now the shader is active

      vao.bind();
      glDrawArrays(GL_PATCHES, 0, 4 * rez * rez);

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

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
  scr_width = width;
  scr_height = height;
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  camera.processMovement(window);
}

void cursorPosCallback(GLFWwindow* window, double xposin, double yposin)
{
  float xpos = static_cast<float>(xposin);
  float ypos = static_cast<float>(yposin);
  camera.updateView(xpos, ypos);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
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

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  float yoff = static_cast<float>(yoffset);
  camera.updateZoom(yoff);
}
