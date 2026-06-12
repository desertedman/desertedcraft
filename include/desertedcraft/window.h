#pragma once

#include "gamestate.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h> // Include GLAD before GLFW
#include <stdexcept>

constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;

// Responsible for windowing and input
class Window {
public:
  Window(GameState &gamestate) : mpWindow(nullptr), mGameState(gamestate) {
    mpWindow =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "desertedcraft", NULL, NULL);

    if (!mpWindow) {
      glfwTerminate();
      throw std::runtime_error("Failed to create GLFW window");
    }

    // Set callback functions
    glfwMakeContextCurrent(mpWindow);
    glfwSetWindowUserPointer(mpWindow,
                             this); // Set manual pointer to this object
    glfwSetFramebufferSizeCallback(mpWindow, FramebufferSizeCallback);
    glfwSetInputMode(mpWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(mpWindow, MouseCallback);
    // glfwSetScrollCallback(pWindow, scroll_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      throw std::runtime_error("Failed to initialize GLAD");
    }

    // Configure OpenGL
    glEnable(GL_DEPTH_TEST);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(mpWindow, &fbWidth,
                           &fbHeight); // Get pixel coordinates of framebuffer
    glViewport(
        0, 0, fbWidth,
        fbHeight); // Input pixel coordinates, rather than screen coordinates
  }

  void ProcessInput() {
    if (glfwGetKey(mpWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      glfwSetWindowShouldClose(mpWindow, true);
    }

    if (glfwGetKey(mpWindow, GLFW_KEY_W) == GLFW_PRESS)
    {

    }
  }

private:
  static void FramebufferSizeCallback(GLFWwindow *window, int width,
                                      int height) {
    glViewport(0, 0, width, height);
  }
  static void MouseCallback(GLFWwindow *window, double xpos, double ypos);

  GLFWwindow *mpWindow;
  GameState &mGameState;
};
