#include "gamestate.h"
#include "chunkmanager.h"
#include "window.h"
// #include <iostream>

GameState::GameState()
    : mLastX((float)SCR_WIDTH / 2), mLastY((float)SCR_HEIGHT / 2),
      mDeltaTime(0.f), mLastFrame(0.f), mFirstMouse(true),
      mCamera(glm::vec3(0.f, 0.f, 3.f)), mCaptureMouse(true),
      chunksLoadedList(this) {
  // int ret = chunksLoadedList.InitChunks();
  // if (ret != 0)
  //   std::cerr << "FAILED TO INIT CHUNKS\n";
}

void GameState::Update() {
  float currentFrame = static_cast<float>(glfwGetTime());
  mDeltaTime = currentFrame - mLastFrame;
  mLastFrame = currentFrame;
}

// TODO: Move this behavior into Callbacks::MouseCallback?
void GameState::ProcessMouseCallback(double xpos, double ypos) {
  if (mCaptureMouse == true) {
    // Camera rotation
    if (mFirstMouse) {
      mLastX = xpos;
      mLastY = ypos;
      mFirstMouse = false;
    }

    float xoffset = xpos - mLastX;
    float yoffset =
        // reversed since y-coordinates go from bottom to top
        mLastY - ypos;

    mLastX = xpos;
    mLastY = ypos;

    mCamera.ProcessMouseMovement(xoffset, yoffset);
  }
}

const Camera &GameState::GetConstCamera() const { return mCamera; }

Camera &GameState::GetCamera() const {
  auto &camera = GetConstCamera();
  return const_cast<Camera &>(camera);
}

void GameState::SendInputEvent(Camera_Movement movement) {
  mCamera.ProcessKeyboard(movement, mDeltaTime);
}

void GameState::SetCaptureMouse(const bool mode) {
  mCaptureMouse = mode;
  // Fixes mouse spazzing on recapturing mouse
  mFirstMouse = true;
}
