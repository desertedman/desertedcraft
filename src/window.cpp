#include "window.h"
#include <GLFW/glfw3.h>
#include <iostream>

Window::Window(GameState &gamestate, GLFWwindow *windowPtr)
    : m_windowPtr(windowPtr), m_gameState(gamestate), m_shouldCaptureMouse(true) {
  glfwMakeContextCurrent(m_windowPtr);
  glfwSetWindowUserPointer(m_windowPtr,
                           this); // Set manual pointer to this object
  glfwSetFramebufferSizeCallback(m_windowPtr,
                                 Callbacks::FramebufferSizeCallback);
  glfwSetInputMode(m_windowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(m_windowPtr, Callbacks::MouseCallback);
  glfwSetKeyCallback(m_windowPtr, Callbacks::KeyCallback);
  // glfwSetScrollCallback(pWindow, scroll_callback);
}

const GLFWwindow *Window::GetWindowPtr() const { return m_windowPtr; }

const int Window::ShouldWindowClose() const {
  return glfwWindowShouldClose(m_windowPtr);
}

void Window::Update() {
  glfwSwapBuffers(m_windowPtr);
  glfwPollEvents();
}

void Window::ProcessInput() {
  if (glfwGetKey(m_windowPtr, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(m_windowPtr, true);

  // if (glfwGetKey(mWindowPtr, GLFW_KEY_E) == GLFW_PRESS)
  //   ToggleMouseCapture();

  if (glfwGetKey(m_windowPtr, GLFW_KEY_W) == GLFW_PRESS)
    m_gameState.SendInputEvent(Camera_Movement::FORWARD);
  if (glfwGetKey(m_windowPtr, GLFW_KEY_S) == GLFW_PRESS)
    m_gameState.SendInputEvent(Camera_Movement::BACKWARD);
  if (glfwGetKey(m_windowPtr, GLFW_KEY_A) == GLFW_PRESS)
    m_gameState.SendInputEvent(Camera_Movement::LEFT);
  if (glfwGetKey(m_windowPtr, GLFW_KEY_D) == GLFW_PRESS)
    m_gameState.SendInputEvent(Camera_Movement::RIGHT);
  if (glfwGetKey(m_windowPtr, GLFW_KEY_SPACE) == GLFW_PRESS)
    m_gameState.SendInputEvent(Camera_Movement::UP);
  if (glfwGetKey(m_windowPtr, GLFW_KEY_C) == GLFW_PRESS)
    m_gameState.SendInputEvent(Camera_Movement::DOWN);
}

void Window::ToggleMouseCapture() {
  // Toggle mouse capture off
  if (m_shouldCaptureMouse == true) {
    glfwSetInputMode(m_windowPtr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    m_shouldCaptureMouse = false;
    std::cout << "Capturing mouse: OFF\n";
  }

  // Toggle mouse capture on
  else {
    glfwSetInputMode(m_windowPtr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    m_shouldCaptureMouse = true;
    std::cout << "Capturing mouse: ON\n";
  }

  m_gameState.SetCaptureMouse(m_shouldCaptureMouse);
}

// mode = GLFW_CURSOR_DISABLED or GLFW_CURSOR_NORMAL
void Window::SetCursorMode(int mode) {
  if (mode == GLFW_CURSOR_DISABLED)
    m_shouldCaptureMouse = true;

  else if (mode == GLFW_CURSOR_NORMAL)
    m_shouldCaptureMouse = false;

  m_gameState.SetCaptureMouse(m_shouldCaptureMouse);
  glfwSetInputMode(m_windowPtr, GLFW_CURSOR, mode);
}

void Callbacks::FramebufferSizeCallback(GLFWwindow *window, int width,
                                        int height) {
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
void Callbacks::KeyCallback(GLFWwindow *window, int key, int scancode,
                            int action, int mods) {
  auto windowPtr = static_cast<Window *>(glfwGetWindowUserPointer(window));

  if (windowPtr) {
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
      windowPtr->ToggleMouseCapture();
  }
}
