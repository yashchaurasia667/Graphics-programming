#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>
#include <camera.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800, SCR_HEIGHT = 600;

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "lighting", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create an opengl window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to load opengl headers" << std::endl;
    glfwSetWindowShouldClose(window, true);
    return -1;
  }

  Shader s("../shaders/vertex.glsl", "../shaders/fragment.glsl");
  Camera c(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), window);
  float vertices[] = {
      // position         // color
      // FRONT
      -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, // bottom left
      0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,  // bottom right
      0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,   // top right
      -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,  // top left

      // BACK
      -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, // bottom left
      0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f,  // bottom right
      0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f,   // top right
      -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f   // top left
  };
  unsigned int indices[] = {
      0, 1, 2,
      0, 3, 2,

      4, 5, 6,
      4, 7, 6,

      4, 7, 3,
      4, 0, 3,

      5, 6, 2,
      5, 1, 2,

      4, 5, 1,
      4, 0, 1,

      6, 7, 3,
      6, 2, 3};

  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // position attributes
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // color attributes
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glEnable(GL_DEPTH_TEST);

  while (!glfwWindowShouldClose(window))
  {
    processInput(window);
    c.processMovement(window, 2.5f);
    glClearColor(0.4f, 0.4f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection;

    model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.1f, 1.0f, 0.0f));
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    s.setMat4("model", model);
    s.setMat4("view", view);
    s.setMat4("projection", projection);

    s.use();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}