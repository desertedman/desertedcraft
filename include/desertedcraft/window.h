#pragma once

#include "camera.h"
#include "gamestate.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h> // Include GLAD before GLFW

constexpr unsigned int SCR_WIDTH_TEMP = 1280;
constexpr unsigned int SCR_HEIGHT_TEMP = 720;

// Responsible for windowing and input
class Window {
public:
  // NOTE: I would much prefer to keep all OpenGL and window initialization
  // logic kept in one place; particularly in the Application constructor. This
  // seems like a recipe for a disaster, having to hunt for bugs in
  // initialization logic all over the place.
  // Instead, what I could do is once we've finished initialization in
  // Application(), pass the window pointer to this Window class ala dependency
  // injection, and then any further calls regarding the window ptr should be
  // handled through this Window class.
  Window(GameState &gamestate, GLFWwindow *windowPtr)
      : mWindowPtr(nullptr), mGameState(gamestate) {}

  void Update() {
    float currentFrame = static_cast<float>(glfwGetTime());
    mDeltaTime = currentFrame - mLastFrame;
    mLastFrame = currentFrame;
  }

  // NOTE: Window should accept input events, but it should NOT be telling what
  // those inputs mean. Those should be left to the GameState, to mediate
  // between the Window class and any other controllers, such as the Camera
  // class.
  void ProcessInput() {
    if (glfwGetKey(mWindowPtr, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(mWindowPtr, true);
    }

    // Camera movement
    auto &camera = mGameState.GetCamera();
    if (glfwGetKey(mWindowPtr, GLFW_KEY_W) == GLFW_PRESS)
      camera.ProcessKeyboard(Camera_Movement::FORWARD, mDeltaTime);
    if (glfwGetKey(mWindowPtr, GLFW_KEY_S) == GLFW_PRESS)
      camera.ProcessKeyboard(Camera_Movement::BACKWARD, mDeltaTime);
    if (glfwGetKey(mWindowPtr, GLFW_KEY_A) == GLFW_PRESS)
      camera.ProcessKeyboard(Camera_Movement::LEFT, mDeltaTime);
    if (glfwGetKey(mWindowPtr, GLFW_KEY_D) == GLFW_PRESS)
      camera.ProcessKeyboard(Camera_Movement::RIGHT, mDeltaTime);
    if (glfwGetKey(mWindowPtr, GLFW_KEY_SPACE) == GLFW_PRESS)
      camera.ProcessKeyboard(Camera_Movement::UP, mDeltaTime);
    if (glfwGetKey(mWindowPtr, GLFW_KEY_C) == GLFW_PRESS)
      camera.ProcessKeyboard(Camera_Movement::DOWN, mDeltaTime);
  }

  const GameState &GetGameStateRef() const { return mGameState; }

private:
  static void FramebufferSizeCallback(GLFWwindow *window, int width,
                                      int height) {
    glViewport(0, 0, width, height);
  }

  static void MouseCallback(GLFWwindow *window, double xpos, double ypos) {
    // Get manual user ptr from earlier glfwSetWindowUserPointer
    auto windowPtr = static_cast<Window *>(glfwGetWindowUserPointer(window));

    if (windowPtr) {
      // GameState needs to mediate this, because this function is static; we
      // can only access static variables in this function
      windowPtr->mGameState.ProcessMouseCallback(xpos, ypos);
    }
  }

  float mDeltaTime;
  float mLastFrame;

  GLFWwindow *mWindowPtr;
  GameState &mGameState;
};
