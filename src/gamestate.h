#pragma once

#include "camera.h"
#include "chunkmanager.h"
#include <GLFW/glfw3.h>

class GameState {
public:
  GameState();
  GameState(const GameState &other) = delete;
  GameState(const GameState &&other) = delete;
  GameState &operator=(const GameState &other) = delete;
  GameState &operator=(const GameState &&other) = delete;

  void Update();
  void ProcessMouseCallback(double xpos, double ypos);
  const Camera &GetConstCamera() const;
  Camera &GetCamera() const;
  const glm::ivec3 GetPlayerChunkCoords() const {
    const auto pos = GetConstCamera().Position;
    const auto retPos = ChunkManager::WorldToChunkCoords(pos);

    return retPos;
  }
  void SendInputEvent(Camera_Movement movement);
  void SetCaptureMouse(const bool mode);

  ChunkManager chunkManager;

private:
  Camera m_camera;

  // Mouse
  // NOTE: Uncomfortable with this being here. Investigate moving to Callbacks
  // namespace
  float m_lastX;
  float m_lastY;
  bool m_firstMouse;
  bool m_captureMouse;

  // Delta Time
  float m_deltaTime;
  float m_lastFrame;
};
