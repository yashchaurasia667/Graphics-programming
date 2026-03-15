#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include <camera.h>
#include <model.h>
#include <shader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <utils.h>

unsigned int scr_width = 1280, scr_height = 720;
float deltaTime = 0.0f, lastFrame = 0.0f;
bool firstMouse = true;

Camera camera;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void camera_callback(GLFWwindow *window, double xposIn, double yposIn);
void zoom_callback(GLFWwindow *window, double xoffset, double yoffset);
unsigned int loadTexture(char const *path, bool gammaCorrection);
void process_input(GLFWwindow *window);

void renderQuad();
void renderCube();

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(scr_width, scr_height,
                                        "deferred shading", nullptr, nullptr);
  if (!window) {
    std::cout << "Failed to create a GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to load opengl function pointers" << std::endl;
    return -1;
  }
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, camera_callback);
  glfwSetScrollCallback(window, zoom_callback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glEnable(GL_DEPTH_TEST);

  camera = Camera(glm::vec3(0.0f), 45.0f, window, scr_width, scr_height);

  // Shaders
  Shader geometryPassShader("../shaders/g_buffer.vs", "../shaders/g_buffer.fs");
  Shader lightingPassShader("../shaders/deferred_shading.vs",
                            "../shaders/deferred_shading.fs");
  Shader lightBoxShader("../shaders/deferred_light_box.vs",
                        "../shaders/deferred_light_box.fs");

  Model backpack("../../resources/objects/backpack.obj");
  std::vector<glm::vec3> objectPositions;
  objectPositions.push_back(glm::vec3(-3.0, -0.5, -3.0));
  objectPositions.push_back(glm::vec3(0.0, -0.5, -3.0));
  objectPositions.push_back(glm::vec3(3.0, -0.5, -3.0));
  objectPositions.push_back(glm::vec3(-3.0, -0.5, 0.0));
  objectPositions.push_back(glm::vec3(0.0, -0.5, 0.0));
  objectPositions.push_back(glm::vec3(3.0, -0.5, 0.0));
  objectPositions.push_back(glm::vec3(-3.0, -0.5, 3.0));
  objectPositions.push_back(glm::vec3(0.0, -0.5, 3.0));
  objectPositions.push_back(glm::vec3(3.0, -0.5, 3.0));

  // lighting info
  const unsigned int NR_LIGHTS = 32;
  std::vector<glm::vec3> lightPositions;
  std::vector<glm::vec3> lightColors;
  srand(13);
  for (unsigned int i = 0; i < NR_LIGHTS; i++) {
    float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
    float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
    float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
    lightPositions.push_back(glm::vec3(xPos, yPos, zPos));

    float rColor = static_cast<float>(((rand() % 100) / 200.0f) +
                                      0.5); // between 0.5 and 1.0
    float gColor = static_cast<float>(((rand() % 100) / 200.0f) +
                                      0.5); // between 0.5 and 1.0
    float bColor = static_cast<float>(((rand() % 100) / 200.0f) +
                                      0.5); // between 0.5 and 1.0
    lightColors.push_back(glm::vec3(rColor, gColor, bColor));
  }

  unsigned int gBuffer;
  glGenFramebuffers(1, &gBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

  // stuff to store in the gBuffer
  // position
  // albedo
  // normal map
  // specular map
  // light pos[]
  // view pos
  unsigned int gPos, gNormal, gAlbedoSpec;

  // position buffer
  glGenTextures(1, &gPos);
  glBindTexture(GL_TEXTURE_2D, gPos);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, scr_width, scr_height, 0, GL_RGBA,
               GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         gPos, 0);
  // normal color buffer
  glGenTextures(1, &gNormal);
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, scr_width, scr_height, 0, GL_RGBA,
               GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         gNormal, 0);
  // color + specular buffer
  glGenTextures(1, &gAlbedoSpec);
  glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scr_width, scr_height, 0, GL_RGBA,
               GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                         gAlbedoSpec, 0);

  unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                 GL_COLOR_ATTACHMENT2};
  glDrawBuffers(3, attachments);

  unsigned int rboDepth;
  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scr_width,
                        scr_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rboDepth);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer incomplete!!" << std::endl;
    return -1;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  while (!glfwWindowShouldClose(window)) {
    float currFrame = glfwGetTime();
    deltaTime = currFrame - lastFrame;
    lastFrame = currFrame;

    process_input(window);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Geometry pass
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera.get_view_matrix();
    glm::mat4 projection =
        glm::perspective(glm::radians(camera.get_fov()),
                         (float)scr_width / (float)scr_height, 0.1f, 100.0f);

    geometryPassShader.use();
    geometryPassShader.setMat4("view", view);
    geometryPassShader.setMat4("projection", projection);

    // configure shader transforms and uniforms
    for (glm::vec3 pos : objectPositions) {
      model = glm::mat4(1.0f);
      model = glm::translate(model, pos);
      model = glm::scale(model, glm::vec3(0.5f));
      geometryPassShader.setMat4("model", model);

      backpack.draw(geometryPassShader);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. Lighting pass
    glDisable(GL_DEPTH_TEST);

    lightingPassShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPos);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

    // set lighting uniforms
    lightingPassShader.setInt("gPosition", 0);
    lightingPassShader.setInt("gNormal", 1);
    lightingPassShader.setInt("gAlbedoSpec", 2);

    for (int i = 0; i < NR_LIGHTS; i++) {
      model = glm::mat4(1.0f);
      model = glm::scale(model, glm::vec3(0.3f));

      lightingPassShader.setVec3("lights[" + std::to_string(i) + "].Position",
                                 lightPositions[i]);
      lightingPassShader.setVec3("lights[" + std::to_string(i) + "].Color",
                                 lightColors[i]);
      const float linear = 0.7f;
      const float quadratic = 1.8f;
      lightingPassShader.setFloat("lights[" + std::to_string(i) + "].Linear",
                                  linear);
      lightingPassShader.setFloat("lights[" + std::to_string(i) + "].Quadratic",
                                  quadratic);
    }
    lightingPassShader.setVec3("viewPos", camera.get_pos());

    renderQuad();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBlitFramebuffer(0, 0, scr_width, scr_height, 0, 0, scr_width, scr_height,
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);

    lightBoxShader.use();
    lightBoxShader.setMat4("projection", projection);
    lightBoxShader.setMat4("view", camera.get_view_matrix());
    for (unsigned int i = 0; i < lightPositions.size(); i++) {
      model = glm::mat4(1.0f);
      model = glm::translate(model, lightPositions[i]);
      model = glm::scale(model, glm::vec3(0.125f));

      lightBoxShader.setMat4("model", model);
      lightBoxShader.setVec3("lightColor", lightColors[i]);
      renderCube();
    }

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glfwTerminate();
  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  scr_width = width;
  scr_height = height;
  glViewport(0, 0, width, height);
}

void camera_callback(GLFWwindow *window, double xposIn, double yposIn) {
  camera.mouse_callback(window, xposIn, yposIn, &firstMouse, 0.1f);
}

void zoom_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.scroll_callback(window, xoffset, yoffset);
}

void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  else {
    glm::vec3 pos = camera.get_pos();
    std::cout << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
    camera.process_movement(window, 2.5f, deltaTime);
  }
}