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
  void ToggleMouseCapture();
  void SetCursorMode(int mode);
  GameState &GetGameState() const { return mGameState; }

private:
  bool mShouldCaptureMouse;
  GLFWwindow *mWindowPtr;
  GameState &mGameState;
};

namespace Callbacks {
void FramebufferSizeCallback(GLFWwindow *window, int width, int height);
void MouseCallback(GLFWwindow *window, double xpos, double ypos);
void KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods);
} // namespace Callbacks
