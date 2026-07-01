#include "application.h"
#include "camera.h"
#include "chunkmanager.h"
#include "gamestate.h"
#include "glad/glad.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "renderer.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <cassert>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

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
  auto &chunkManager = mGameStatePtr->chunkManager;
  chunkManager.Update();
  const auto &meshes = chunkManager.GetChunksRenderList().GetMeshes();

  std::atomic_bool chunksListDirty = false;
  std::atomic_bool dispatchRunning = true;
  std::mutex renderMutex;
  std::thread dispatch(&ChunkManager::Dispatch, &chunkManager,
                       std::ref(renderMutex), std::ref(chunksListDirty),
                       std::ref(dispatchRunning));

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode

  while (!mWindowWrapperPtr->ShouldWindowClose()) {
    mGameStatePtr->Update(); // Update delta time
    mWindowWrapperPtr->ProcessInput();

    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render meshes
    if (chunksListDirty) {
      std::lock_guard<std::mutex> renderGuard(renderMutex);
      chunkManager.UpdateChunksRenderList();
      chunksListDirty = false;
    }

    for (int i = 0; i < meshes.size(); i++) {
      const auto &transform =
          chunkManager.GetChunksRenderList().GetChunkWorldCoordsList()[i];
      mRendererPtr->Draw(meshes[i].get(), static_cast<int>(transform.x),
                         static_cast<int>(transform.y),
                         static_cast<int>(transform.z));
    }

    const auto &playerWorldCoords = mGameStatePtr.get()->GetCamera().Position;
    const auto &playerChunkCoords =
        ChunkManager::WorldToChunkCoords(playerWorldCoords);
    // Construct coordinate strings
    std::string worldCoords = "X: " + std::to_string(playerWorldCoords.x) +
                              " Y: " + std::to_string(playerWorldCoords.y) +
                              " Z: " + std::to_string(playerWorldCoords.z);
    std::string chunkCoords = "X: " + std::to_string(playerChunkCoords.x) +
                              " Y: " + std::to_string(playerChunkCoords.y) +
                              " Z: " + std::to_string(playerChunkCoords.z);

    // ImGui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Text("World coordinates");
    ImGui::Text("%s", worldCoords.c_str());
    ImGui::Text("Chunk coordinates");
    ImGui::Text("%s", chunkCoords.c_str());
    // bool showWindow = true;
    // ImGui::ShowDemoWindow(&showWindow);
    //
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    mWindowWrapperPtr->Update();
  }

  dispatchRunning = false;
  dispatch.join();
}
