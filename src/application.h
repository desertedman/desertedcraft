#pragma once

#include "glad/glad.h"
#include "renderer.h"
#include <GLFW/glfw3.h> // Include GLAD before GLFW

class Application {
public:
  Application();
  ~Application();

  void Run();

private:
  Renderer *pRenderer;
  GLFWwindow *pWindow;
};
