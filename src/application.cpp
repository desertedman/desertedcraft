#include "application.h"
#include "block.h"
#include "camera.h"
#include "renderer.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

Application::Application()
    : pWindow(nullptr), pRenderer(nullptr), lastX((float)SCR_WIDTH / 2),
      lastY((float)SCR_HEIGHT / 2), deltaTime(0.f), lastFrame(0.f),
      firstMouse(true), mCamera(glm::vec3(0.f, 0.f, 3.f)) {
  // Initialize GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  pWindow =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "desertedcraft", NULL, NULL);

  if (!pWindow) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwMakeContextCurrent(pWindow);
  glfwSetWindowUserPointer(pWindow, this);
  glfwSetFramebufferSizeCallback(pWindow, FramebufferSizeCallback);
  glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(pWindow, MouseCallback);
  // glfwSetScrollCallback(pWindow, scroll_callback);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw std::runtime_error("Failed to initialize GLAD");
  }

  glEnable(GL_DEPTH_TEST);
  glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode

  pRenderer = new Renderer(mCamera);
}

Application::~Application() {
  delete pRenderer;
  pRenderer = nullptr;

  glfwTerminate();
}

void Application::Run() {
  Block block;

  // TODO: Process game state and render state independently
  while (!glfwWindowShouldClose(pWindow)) {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    ProcessInput();

    // glClearColor(0.2f, 0.3f, 0.3f, 1.f);
    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw stuff...
    pRenderer->Draw(block);

    glfwSwapBuffers(pWindow);
    glfwPollEvents();
  }
}

void Application::ProcessInput() {
  if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(pWindow, true);
  }

  // Camera movement
  if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
    mCamera.ProcessKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
    mCamera.ProcessKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
    mCamera.ProcessKeyboard(LEFT, deltaTime);
  if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
    mCamera.ProcessKeyboard(RIGHT, deltaTime);
  if (glfwGetKey(pWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
    mCamera.ProcessKeyboard(UP, deltaTime);
  if (glfwGetKey(pWindow, GLFW_KEY_C) == GLFW_PRESS)
    mCamera.ProcessKeyboard(DOWN, deltaTime);
}

void Application::FramebufferSizeCallback(GLFWwindow *window, int width,
                                            int height) {
  glViewport(0, 0, width, height);
}

void Application::MouseCallback(GLFWwindow *window, double xpos, double ypos) {
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->ProcessMouseCallback(xpos, ypos);
  }

  else {
    std::cerr << "Mouse callback: nullptr\n";
  }
}

void Application::ProcessMouseCallback(double xpos, double ypos) {
  // Camera rotation
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset =
      lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  mCamera.ProcessMouseMovement(xoffset, yoffset);
}
