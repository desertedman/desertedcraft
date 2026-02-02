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

  glfwMakeContextCurrent(m_pWindow);
  glfwSetWindowUserPointer(m_pWindow, this);
  glfwSetFramebufferSizeCallback(m_pWindow, FramebufferSizeCallback);
  glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(m_pWindow, MouseCallback);
  // glfwSetScrollCallback(pWindow, scroll_callback);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw std::runtime_error("Failed to initialize GLAD");
  }

  glEnable(GL_DEPTH_TEST);
  glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode

  m_pGameState = std::make_unique<GameState>(*m_pWindow);
  m_pRenderer = std::make_unique<Renderer>(m_pGameState->mCamera);

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
  std::array<std::array<std::array<Block, arraySize>, arraySize>, arraySize>
      Blocks;

  // TODO: Process game state and render state independently
  while (!glfwWindowShouldClose(m_pWindow)) {
    m_pGameState->Update();

    m_pGameState->ProcessInput();

    // glClearColor(0.2f, 0.3f, 0.3f, 1.f);
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
