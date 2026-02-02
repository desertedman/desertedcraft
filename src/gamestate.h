#pragma once

#include "block.h"
#include "camera.h"
#include <GLFW/glfw3.h>
#include <array>

constexpr int arraySize = 16;

class GameState {
public:
  GameState(GLFWwindow &window);
  void Update();
  void ProcessInput();
  void ProcessMouseCallback(double xpos, double ypos);
  const Camera &GetCamera() const;

  std::array<std::array<std::array<Block, arraySize>, arraySize>, arraySize>
      m_pBlocks;

private:
  GLFWwindow &mWindow;
  Camera mCamera;
  float mLastX;
  float mLastY;
  bool mFirstMouse;
  float mDeltaTime;
  float mLastFrame;
};
