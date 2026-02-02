#pragma once

#include "camera.h"
// #include "gamestate.h"
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

  // GameState mGameState;

  Renderer *m_pRenderer;
  GLFWwindow *m_pWindow;

  Camera mCamera;
  float mLastX;
  float mLastY;
  bool mFirstMouse;
  float mDeltaTime;
  float mLastFrame;
};
