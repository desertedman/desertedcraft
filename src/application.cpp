#include "application.h"
#include "camera.h"
#include "chunk.h"
#include "gamestate.h"
#include "glad/glad.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "mesher_naive.h"
#include "renderer.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <stdexcept>

Application::Application() {
  // Initialize GLFW
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *windowPtr =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "desertedcraft", NULL, NULL);

  if (!windowPtr) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwMakeContextCurrent(windowPtr);

  // Initialize GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw std::runtime_error("Failed to initialize GLAD");
  }

  // Configure OpenGL
  glEnable(GL_DEPTH_TEST);
  int fbWidth, fbHeight;
  // Get pixel coordinates of framebuffer
  glfwGetFramebufferSize(windowPtr, &fbWidth, &fbHeight);
  // Input pixel coordinates, rather than screen coordinates
  glViewport(0, 0, fbWidth, fbHeight);

  mGameStatePtr = std::make_unique<GameState>();
  mRendererPtr = std::make_unique<Renderer>(mGameStatePtr->GetConstCamera());

  if (!mRendererPtr) {
    glfwTerminate();
    throw std::runtime_error("Failed to create Renderer");
  }

  else if (!mGameStatePtr) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GameState");
  }

  // constructor sets callback functions
  // mWindowWrapperPtr is not responsible for mGameStatePtr's lifetime, but we
  // need a ptr to it
  mWindowWrapperPtr = std::make_unique<Window>(*mGameStatePtr.get(), windowPtr);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  ImGui::StyleColorsDark();
  ImGuiStyle &style = ImGui::GetStyle();

  ImGui_ImplGlfw_InitForOpenGL(windowPtr, true);
  const char *glsl_version = "#version 130";
  ImGui_ImplOpenGL3_Init(glsl_version);
}

Application::~Application() { glfwTerminate(); }

void Application::Run() {
  // TODO: Process game state and render state independently

  // Create mesher
  [[maybe_unused]] MesherNaive mesher;
  // MesherBasic mesher;

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode

  while (!mWindowWrapperPtr->ShouldWindowClose()) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    bool showWindow = true;
    ImGui::ShowDemoWindow(&showWindow);

    mGameStatePtr->Update(); // Update delta time
    mWindowWrapperPtr->ProcessInput();

    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Create mesh
    DrawableMesh mesh = mesher.CreateMesh(mGameStatePtr->mChunk.GetBlocksPtr());
    mRendererPtr->Draw(&mesh);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    mWindowWrapperPtr->Update();
  }
}
