#include "gamestate.h"
#include "chunkmanager.h"
#include "window.h"
// #include <iostream>

GameState::GameState()
    : m_lastX((float)SCR_WIDTH / 2), m_lastY((float)SCR_HEIGHT / 2),
      m_deltaTime(0.f), m_lastFrame(0.f), m_firstMouse(true),
      m_camera(glm::vec3(0.f, 0.f, 0.f)), m_captureMouse(true),
      chunkManager(*this) {}

void GameState::Update() {
  float currentFrame = static_cast<float>(glfwGetTime());
  m_deltaTime = currentFrame - m_lastFrame;
  m_lastFrame = currentFrame;
}

// TODO: Move this behavior into Callbacks::MouseCallback?
void GameState::ProcessMouseCallback(double xpos, double ypos) {
  if (m_captureMouse == true) {
    // Camera rotation
    if (m_firstMouse) {
      m_lastX = static_cast<float>(xpos);
      m_lastY = static_cast<float>(ypos);
      m_firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - m_lastX);
    float yoffset =
        // reversed since y-coordinates go from bottom to top
        static_cast<float>(m_lastY - ypos);

    m_lastX = static_cast<float>(xpos);
    m_lastY = static_cast<float>(ypos);

    m_camera.ProcessMouseMovement(xoffset, yoffset);
  }
}

const Camera &GameState::GetConstCamera() const { return m_camera; }

Camera &GameState::GetCamera() const {
  auto &camera = GetConstCamera();
  return const_cast<Camera &>(camera);
}

void GameState::SendInputEvent(Camera_Movement movement) {
  m_camera.ProcessKeyboard(movement, m_deltaTime);
}

void GameState::SetCaptureMouse(const bool mode) {
  m_captureMouse = mode;
  // Fixes mouse spazzing on recapturing mouse
  m_firstMouse = true;
}
