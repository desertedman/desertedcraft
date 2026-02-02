#include "gamestate.h"
#include "renderer.h"

GameState::GameState(GLFWwindow &window)
    : mLastX((float)SCR_WIDTH / 2), mLastY((float)SCR_HEIGHT / 2),
      mDeltaTime(0.f), mLastFrame(0.f), mFirstMouse(true),
      mCamera(glm::vec3(0.f, 0.f, 3.f)), mWindow(window) {}

void GameState::Update() {
  float currentFrame = static_cast<float>(glfwGetTime());
  mDeltaTime = currentFrame - mLastFrame;
  mLastFrame = currentFrame;
}

void GameState::ProcessInput() {
  if (glfwGetKey(&mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(&mWindow, true);
  }

  // Camera movement
  if (glfwGetKey(&mWindow, GLFW_KEY_W) == GLFW_PRESS)
    mCamera.ProcessKeyboard(Camera_Movement::FORWARD, mDeltaTime);
  if (glfwGetKey(&mWindow, GLFW_KEY_S) == GLFW_PRESS)
    mCamera.ProcessKeyboard(Camera_Movement::BACKWARD, mDeltaTime);
  if (glfwGetKey(&mWindow, GLFW_KEY_A) == GLFW_PRESS)
    mCamera.ProcessKeyboard(Camera_Movement::LEFT, mDeltaTime);
  if (glfwGetKey(&mWindow, GLFW_KEY_D) == GLFW_PRESS)
    mCamera.ProcessKeyboard(Camera_Movement::RIGHT, mDeltaTime);
  if (glfwGetKey(&mWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
    mCamera.ProcessKeyboard(Camera_Movement::UP, mDeltaTime);
  if (glfwGetKey(&mWindow, GLFW_KEY_C) == GLFW_PRESS)
    mCamera.ProcessKeyboard(Camera_Movement::DOWN, mDeltaTime);
}

void GameState::ProcessMouseCallback(double xpos, double ypos) {
  // Camera rotation
  if (mFirstMouse) {
    mLastX = xpos;
    mLastY = ypos;
    mFirstMouse = false;
  }

  float xoffset = xpos - mLastX;
  float yoffset =
      mLastY - ypos; // reversed since y-coordinates go from bottom to top

  mLastX = xpos;
  mLastY = ypos;

  mCamera.ProcessMouseMovement(xoffset, yoffset);
}

const Camera &GameState::GetCamera() const { return mCamera; }
