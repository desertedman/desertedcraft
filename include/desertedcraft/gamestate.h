#pragma once

#include "camera.h"
#include "chunkmanager.h"
#include <GLFW/glfw3.h>

class GameState {
public:
  GameState();
  void Update();
  void ProcessMouseCallback(double xpos, double ypos);
  const Camera &GetConstCamera() const;
  Camera &GetCamera() const;
  void SendInputEvent(Camera_Movement movement);
  void SetCaptureMouse(const bool mode);

  ChunksLoadedList chunksLoadedList;

private:
  Camera mCamera;

  // Mouse
  // NOTE: Uncomfortable with this being here. Investigate moving to Callbacks
  // namespace
  float mLastX;
  float mLastY;
  bool mFirstMouse;
  bool mCaptureMouse;

  // Delta Time
  float mDeltaTime;
  float mLastFrame;
};
