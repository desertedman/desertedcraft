#pragma once

#include "camera.h"
#include "glad/glad.h"
#include "renderer.h"
#include <GLFW/glfw3.h> // Include GLAD before GLFW

class Application {
public:
  Application();
  ~Application();

  void Run();

private:
  void ProcessInput();

  Camera mCamera;
  Renderer *pRenderer;
  GLFWwindow *pWindow;

  float deltaTime = 0.f;
  float lastFrame = 0.f;
};
