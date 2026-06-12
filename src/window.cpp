#include "window.h"

Window::Window(GameState &gamestate, GLFWwindow *windowPtr)
    : mWindowPtr(windowPtr), mGameState(gamestate) {
  glfwMakeContextCurrent(mWindowPtr);
  glfwSetWindowUserPointer(mWindowPtr,
                           this); // Set manual pointer to this object
  glfwSetFramebufferSizeCallback(mWindowPtr, FramebufferSizeCallback);
  glfwSetInputMode(mWindowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(mWindowPtr, MouseCallback);
  // glfwSetScrollCallback(pWindow, scroll_callback);
}

const GLFWwindow *Window::GetWindowPtr() const { return mWindowPtr; }

const int Window::ShouldWindowClose() const {
  return glfwWindowShouldClose(mWindowPtr);
}

void Window::Update() {
  glfwSwapBuffers(mWindowPtr);
  glfwPollEvents();
}

void Window::ProcessInput() {
  if (glfwGetKey(mWindowPtr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(mWindowPtr, true);

  if (glfwGetKey(mWindowPtr, GLFW_KEY_W) == GLFW_PRESS)
    mGameState.SendInputEvent(Camera_Movement::FORWARD);
  if (glfwGetKey(mWindowPtr, GLFW_KEY_S) == GLFW_PRESS)
    mGameState.SendInputEvent(Camera_Movement::BACKWARD);
  if (glfwGetKey(mWindowPtr, GLFW_KEY_A) == GLFW_PRESS)
    mGameState.SendInputEvent(Camera_Movement::LEFT);
  if (glfwGetKey(mWindowPtr, GLFW_KEY_D) == GLFW_PRESS)
    mGameState.SendInputEvent(Camera_Movement::RIGHT);
  if (glfwGetKey(mWindowPtr, GLFW_KEY_SPACE) == GLFW_PRESS)
    mGameState.SendInputEvent(Camera_Movement::UP);
  if (glfwGetKey(mWindowPtr, GLFW_KEY_C) == GLFW_PRESS)
    mGameState.SendInputEvent(Camera_Movement::DOWN);
}

void Window::FramebufferSizeCallback(GLFWwindow *window, int width,
                                     int height) {
  glViewport(0, 0, width, height);
}

void Window::MouseCallback(GLFWwindow *window, double xpos, double ypos) {
  // Get manual user ptr from earlier glfwSetWindowUserPointer in constructor
  auto windowPtr = static_cast<Window *>(glfwGetWindowUserPointer(window));

  if (windowPtr) {
    // GameState needs to mediate this, because this function is static; we
    // can only access static variables in this function
    windowPtr->mGameState.ProcessMouseCallback(xpos, ypos);
  }
}
