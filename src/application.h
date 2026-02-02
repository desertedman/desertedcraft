#pragma once

#include "camera.h"
#include "gamestate.h"
#include "glad/glad.h"
#include "renderer.h"
#include <GLFW/glfw3.h> // Include GLAD before GLFW
#include <memory>

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

  // std::unique_ptr<Renderer> m_pRenderer;

  Renderer *m_pRenderer;
  GLFWwindow *m_pWindow;
  GameState *m_pGameState;

  Camera mCamera;
  float mLastX;
  float mLastY;
  bool mFirstMouse;
  float mDeltaTime;
  float mLastFrame;
};
