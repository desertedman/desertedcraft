#pragma once

#include "gamestate.h"

constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;

// Responsible for windowing and input
class Window {
public:
  Window(GameState &gamestate, GLFWwindow *windowPtr);
  const GLFWwindow *GetWindowPtr() const;
  const int ShouldWindowClose() const;
  void Update();
  // Intercepts input from window, which then sends to GameState for processing
  void ProcessInput();  

private:
  static void FramebufferSizeCallback(GLFWwindow *window, int width,
                                      int height);
  static void MouseCallback(GLFWwindow *window, double xpos, double ypos);

  GLFWwindow *mWindowPtr;
  GameState &mGameState;
};
