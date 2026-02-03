#pragma once

#include "camera.h"
#include "chunk.h"
#include <GLFW/glfw3.h>


class GameState {
public:
  GameState(GLFWwindow &window);
  void Update();
  void ProcessInput();
  void ProcessMouseCallback(double xpos, double ypos);
  const Camera &GetCamera() const;

  Chunk mChunk;

private:
  GLFWwindow &mWindow;
  Camera mCamera;
  float mLastX;
  float mLastY;
  bool mFirstMouse;
  float mDeltaTime;
  float mLastFrame;
};
