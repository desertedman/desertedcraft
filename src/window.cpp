#include "window.h"
#include <GLFW/glfw3.h>
#include <iostream>

Window::Window(GameState &gamestate, GLFWwindow *windowPtr)
    : mWindowPtr(windowPtr), mGameState(gamestate), mShouldCaptureMouse(true) {
  glfwMakeContextCurrent(mWindowPtr);
  glfwSetWindowUserPointer(mWindowPtr,
                           this); // Set manual pointer to this object
  glfwSetFramebufferSizeCallback(mWindowPtr,
                                 Callbacks::FramebufferSizeCallback);
  glfwSetInputMode(mWindowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(mWindowPtr, Callbacks::MouseCallback);
  glfwSetKeyCallback(mWindowPtr, Callbacks::KeyCallback);
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

  // if (glfwGetKey(mWindowPtr, GLFW_KEY_E) == GLFW_PRESS)
  //   ToggleMouseCapture();

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

void Window::ToggleMouseCapture() {
  // Toggle mouse capture off
  if (mShouldCaptureMouse == true) {
    glfwSetInputMode(mWindowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    mShouldCaptureMouse = false;
    std::cout << "Capturing mouse: OFF\n";
  }

  // Toggle mouse capture on
  else {
    glfwSetInputMode(mWindowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    mShouldCaptureMouse = true;
    std::cout << "Capturing mouse: ON\n";
  }

  mGameState.SetCaptureMouse(mShouldCaptureMouse);
}

// mode = GLFW_CURSOR_DISABLED or GLFW_CURSOR_NORMAL
void Window::SetCursorMode(int mode) {
  if (mode == GLFW_CURSOR_DISABLED)
    mShouldCaptureMouse = true;

  else if (mode == GLFW_CURSOR_NORMAL)
    mShouldCaptureMouse = false;

  mGameState.SetCaptureMouse(mShouldCaptureMouse);
  glfwSetInputMode(mWindowPtr, GLFW_CURSOR, mode);
}

void Callbacks::FramebufferSizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void Callbacks::MouseCallback(GLFWwindow *window, double xpos, double ypos) {
  // Get manual user ptr from earlier glfwSetWindowUserPointer in constructor
  auto windowPtr = static_cast<Window *>(glfwGetWindowUserPointer(window));

  if (windowPtr) {
    // GameState needs to mediate this, because this function is static; we
    // can only access static variables in this function
    windowPtr->GetGameState().ProcessMouseCallback(xpos, ypos);
  }
}

// Handles singular key press events
void Callbacks::KeyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods) {
  auto windowPtr = static_cast<Window *>(glfwGetWindowUserPointer(window));

  if (windowPtr) {
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
      windowPtr->ToggleMouseCapture();
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
      windowPtr->GetGameState().chunksLoadedList.Update();
  }
}
