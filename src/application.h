#pragma once

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
  static void FramebufferSizeCallback(GLFWwindow *window, int width,
                                      int height);
  static void MouseCallback(GLFWwindow *window, double xpos, double ypos);

  std::unique_ptr<Renderer> m_pRenderer;
  std::unique_ptr<GameState> m_pGameState;

  GLFWwindow *m_pWindow;
};
