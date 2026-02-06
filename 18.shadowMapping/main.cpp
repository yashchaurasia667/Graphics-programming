#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <shader.h>
#include <vertexArray.h>
#include <vertexBuffer.h>
#include <vertexBufferLayout.h>
#include <camera.h>

#include <texture.h>

unsigned int scr_width = 1280, scr_height = 720;
float delta_time, last_frame = 0.0f;
bool first_mouse = true;

Camera camera;

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void cursorPosCallback(GLFWwindow *window, double xposin, double yposin);
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);

float cube_vertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

float plane_vertices[] = {
    // position       // Normal         // texcoords
    -0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,

    -0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window = glfwCreateWindow(scr_width, scr_height, "SHADOW MAPPING", NULL, NULL);
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
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, cursorPosCallback);
  glfwSetScrollCallback(window, scrollCallback);
  glEnable(GL_DEPTH_TEST);

  {
    camera = Camera(glm::vec3(0.0f), 45.0f, window, scr_width, scr_height);
    Shader shader("../shaders/shadowmapping.vs", "../shaders/shadowmapping.fs");
    Shader depth_shader("../shaders/shadow_depth.vs", "../shaders/shadow_depth.fs");

    VertexArray cube_vao;
    VertexBuffer cube_vbo(cube_vertices, 36 * 8, GL_STATIC_DRAW);
    VertexBufferLayout cube_layout;
    cube_layout.push<float>(3);
    cube_layout.push<float>(3);
    cube_layout.push<float>(2);
    cube_vao.addBuffer(cube_vbo, cube_layout);

    VertexArray plane_vao;
    VertexBuffer plane_vbo(plane_vertices, 6 * 8, GL_STATIC_DRAW);
    VertexBufferLayout plane_layout;
    plane_layout.push<float>(3);
    plane_layout.push<float>(3);
    plane_layout.push<float>(2);
    plane_vao.addBuffer(plane_vbo, plane_layout);

    Texture floor_tex("../../assets/metal.png");
    Texture cube_tex("../../assets/container2.png");

    unsigned int depthMapFbo;
    glGenFramebuffers(1, &depthMapFbo);

    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, scr_width, scr_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glm::vec3 lightPos(-12.0f, 10.0f, -1.0f);
    shader.setInt("shadowMap", 1);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
      float curr_frame = glfwGetTime();
      delta_time = curr_frame - last_frame;
      last_frame = curr_frame;

      glfwPollEvents();
      processInput(window);

      glViewport(0, 0, scr_width, scr_height);
      glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
      glClear(GL_DEPTH_BUFFER_BIT);

      glm::mat4 light_proj, light_view;
      glm::mat4 light_space_matrix;
      float near_plane = 1.0f, far_plane = 7.5f;

      light_proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
      light_view = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      light_space_matrix = light_proj * light_view;

      depth_shader.use();
      depth_shader.setMat4("lightSpaceMatrix", light_space_matrix);
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
      model = glm::scale(model, glm::vec3(0.3f));
      depth_shader.setMat4("model", model);

      cube_vao.bind();
      glDrawArrays(GL_TRIANGLES, 0, 36);

      model = glm::mat4(1.0f);
      model = glm::scale(model, glm::vec3(5.0f));
      depth_shader.setMat4("model", model);
      plane_vao.bind();
      glDrawArrays(GL_TRIANGLES, 0, 6);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      {
        glViewport(0, 0, scr_width, scr_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.3f));

        glm::mat4 view = camera.get_view_matrix();
        glm::mat4 projection = glm::perspective(camera.get_fov(), (float)scr_width / (float)scr_height, 0.1f, 100.0f);

        shader.use();
        cube_tex.bind();
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("lightSpaceMatrix", light_space_matrix);

        shader.setVec3("viewPos", camera.get_pos());
        shader.setInt("diffuseTexture", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        shader.setInt("shadowMap", 1);

        cube_vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(5.0f));

        shader.use();
        floor_tex.bind();
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        shader.setMat4("lightSpaceMatrix", light_space_matrix);

        shader.setVec3("viewPos", camera.get_pos());
        shader.setInt("diffuseTexture", 0);

        plane_vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
      }
      glfwSwapBuffers(window);
    }
  }

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
  camera.process_movement(window, 2.5f, delta_time);
}

void cursorPosCallback(GLFWwindow *window, double xposin, double yposin)
{
  camera.mouse_callback(window, xposin, yposin, &first_mouse, 0.1f);
}
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
  camera.scroll_callback(window, xoffset, yoffset);
}