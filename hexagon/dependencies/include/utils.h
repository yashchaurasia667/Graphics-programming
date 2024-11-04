#pragma once

#include <iostream>
#include <vector>

#include <fstream>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void errorCallback(int error, const char *description);
int initGLFW(GLFWwindow **window);
int initGlad(GLFWwindow **window);
void frameBufferCallback(GLFWwindow *window, int width, int height);