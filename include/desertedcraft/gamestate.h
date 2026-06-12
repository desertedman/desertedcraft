#pragma once

#include "camera.h"
#include "chunk.h"
#include <GLFW/glfw3.h>

class GameState {
public:
  GameState();
  void Update();
  void ProcessMouseCallback(double xpos, double ypos);
  const Camera &GetConstCamera() const;
  Camera &GetCamera() const;
  const float getDeltaTime() { return mDeltaTime; }
  void SendInputEvent(Camera_Movement movement);
  Chunk mChunk;

private:
  Camera mCamera;

  // Mouse
  float mLastX;
  float mLastY;
  bool mFirstMouse;

  // Delta Time
  float mDeltaTime;
  float mLastFrame;
};
