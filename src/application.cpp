#include "application.h"
#include "block.h"
#include "camera.h"
#include "gamestate.h"
#include "renderer.h"
#include <GLFW/glfw3.h>
#include <array>
#include <memory>
#include <stdexcept>

Application::Application() : m_pWindow(nullptr) {
  // Initialize GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  m_pWindow =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "desertedcraft", NULL, NULL);

  if (!m_pWindow) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }

  // Set callback functions
  glfwMakeContextCurrent(m_pWindow);
  glfwSetWindowUserPointer(m_pWindow,
                           this); // Set manual pointer to this object
  glfwSetFramebufferSizeCallback(m_pWindow, FramebufferSizeCallback);
  glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(m_pWindow, MouseCallback);
  // glfwSetScrollCallback(pWindow, scroll_callback);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw std::runtime_error("Failed to initialize GLAD");
  }

  // Configure OpenGL
  glEnable(GL_DEPTH_TEST);
  int fbWidth, fbHeight;
  glfwGetFramebufferSize(m_pWindow, &fbWidth,
                         &fbHeight); // Get pixel coordinates of framebuffer
  glViewport(
      0, 0, fbWidth,
      fbHeight); // Input pixel coordinates, rather than screen coordinates
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode

  m_pGameState = std::make_unique<GameState>(*m_pWindow);
  m_pRenderer = std::make_unique<Renderer>(m_pGameState->GetCamera());

  if (!m_pRenderer) {
    glfwTerminate();
    throw std::runtime_error("Failed to create Renderer");
  }

  else if (!m_pGameState) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GameState");
  }
}

Application::~Application() { glfwTerminate(); }

void Application::Run() {
  // std::array<std::array<std::array<Block, arraySize>, arraySize>, arraySize>
  //     Blocks;

  // TODO: Process game state and render state independently
  while (!glfwWindowShouldClose(m_pWindow)) {
    m_pGameState->Update();       // Update delta time
    m_pGameState->ProcessInput(); // Process camera movement

    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw stuff...
    for (int x = 0; x < arraySize; x++)
      for (int y = 0; y < arraySize; y++)
        for (int z = 0; z < arraySize; z++) {
          // Block &currBlock = Blocks[x][y][z];
          Block *currBlock = &(m_pGameState->m_pBlocks[x][y][z]);
          m_pRenderer->Draw(*currBlock, x, y, z);
        }

    glfwSwapBuffers(m_pWindow);
    glfwPollEvents();
  }
}

void Application::FramebufferSizeCallback(GLFWwindow *window, int width,
                                          int height) {
  glViewport(0, 0, width, height);
}

void Application::MouseCallback(GLFWwindow *window, double xpos, double ypos) {
  auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));

  if (app) {
    app->m_pGameState->ProcessMouseCallback(xpos, ypos);
  }
}
