#pragma once

#include "block.h"
#include "camera.h"
#include <GLFW/glfw3.h>
#include <array>

constexpr int arraySize = 16;

class GameState {
public:
  GameState(GLFWwindow &window);
  void Update() {
    float currentFrame = static_cast<float>(glfwGetTime());
    mDeltaTime = currentFrame - mLastFrame;
    mLastFrame = currentFrame;
  }

  void ProcessInput() {
    if (glfwGetKey(&m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(&m_pWindow, true);
    }

    // Camera movement
    if (glfwGetKey(&m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
      mCamera.ProcessKeyboard(FORWARD, mDeltaTime);
    if (glfwGetKey(&m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
      mCamera.ProcessKeyboard(BACKWARD, mDeltaTime);
    if (glfwGetKey(&m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
      mCamera.ProcessKeyboard(LEFT, mDeltaTime);
    if (glfwGetKey(&m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
      mCamera.ProcessKeyboard(RIGHT, mDeltaTime);
    if (glfwGetKey(&m_pWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
      mCamera.ProcessKeyboard(UP, mDeltaTime);
    if (glfwGetKey(&m_pWindow, GLFW_KEY_C) == GLFW_PRESS)
      mCamera.ProcessKeyboard(DOWN, mDeltaTime);
  }

  void ProcessMouseCallback(double xpos, double ypos) {
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

  GLFWwindow &m_pWindow;
  Camera mCamera;
  float mLastX;
  float mLastY;
  bool mFirstMouse;
  float mDeltaTime;
  float mLastFrame;

  std::array<std::array<std::array<Block, arraySize>, arraySize>, arraySize>
      m_pBlocks;

private:
};
