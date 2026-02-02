#include "application.h"
#include "block.h"
#include <GLFW/glfw3.h>
#include <stdexcept>

Application::Application() : pWindow(nullptr), pRenderer(nullptr) {
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

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw std::runtime_error("Failed to initialize GLAD");
  }

  glEnable(GL_DEPTH_TEST);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode

  pRenderer = new Renderer();
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
    // Process input...

    glClearColor(0.2f, 0.3f, 0.3f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw stuff...
    pRenderer->Draw(block);

    glfwSwapBuffers(pWindow);
    glfwPollEvents();
  }
}
