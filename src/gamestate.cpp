#include "gamestate.h"
#include "renderer.h"

GameState::GameState(GLFWwindow &window)
    : mLastX((float)SCR_WIDTH / 2), mLastY((float)SCR_HEIGHT / 2),
      mDeltaTime(0.f), mLastFrame(0.f), mFirstMouse(true),
      mCamera(glm::vec3(0.f, 0.f, 3.f)), m_pWindow(window) {}
