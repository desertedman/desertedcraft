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
  static void FramebufferSizeCallback(GLFWwindow *window, int width,
                                      int height);
  static void MouseCallback(GLFWwindow *window, double xpos, double ypos);
  void ProcessMouseCallback(double xpos, double ypos);

  float deltaTime;
  float lastFrame;

  Renderer *pRenderer;
  GLFWwindow *pWindow;

  Camera mCamera;
  float lastX;
  float lastY;
  bool firstMouse;
};
