#include "application.h"
#include "camera.h"
#include "chunk.h"
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
#include <iostream>
#include <memory>
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

  m_gameStatePtr = std::make_unique<GameState>();
  m_rendererPtr = std::make_unique<Renderer>(m_gameStatePtr->GetConstCamera());

  if (!m_rendererPtr) {
    glfwTerminate();
    throw std::runtime_error("Failed to create Renderer");
  }

  else if (!m_gameStatePtr) {
    glfwTerminate();
    throw std::runtime_error("Failed to create GameState");
  }

  // constructor sets callback functions
  // mWindowWrapperPtr is not responsible for mGameStatePtr's lifetime, but we
  // need a ptr to it
  m_windowWrapperPtr =
      std::make_unique<Window>(*m_gameStatePtr.get(), windowPtr);

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

// TODO: Create some kind of meshing logic
void Application::Run() {
  auto &chunkManager = m_gameStatePtr->chunkManager;
  const auto &renderList = chunkManager.GetChunksRenderList();

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode

  // TODO: Check if this works properly
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  // glFrontFace(GL_CW);

  // Spawn camera at half height
  auto &cameraPos = m_gameStatePtr->GetCamera().Position;
  cameraPos.y = float(CHUNK_SIZE_Y) / 2;

  std::atomic_bool running = true;
  std::thread dispatch(&ChunkManager::Dispatch, &chunkManager,
                       std::ref(running));

  while (!m_windowWrapperPtr->ShouldWindowClose()) {
    m_gameStatePtr->Update(); // Update delta time
    m_windowWrapperPtr->ProcessInput();

    // Constrict camera position to in bounds
    if (cameraPos.y > CHUNK_SIZE_Y - 0.1)
      cameraPos.y = CHUNK_SIZE_Y - 0.1;
    else if (cameraPos.y < 0)
      cameraPos.y = 0;

    chunkManager.Update();

    glClearColor(0.1f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // assert(renderList.empty() != true);
    // int index = 0;
    // while (renderList.empty()) {
    //   std::cout << index << ": RENDER LIST EMPTY\n";
    // }

    for (int i = 0; i < renderList.size(); i++) {
      const auto &transformChunkCoords = renderList[i];
      const auto &chunk = chunkManager.GetChunk(transformChunkCoords);
      const auto *meshPtr = chunk.GetMeshPtr();

      const auto transformWorldCoords =
          ChunkManager::ChunkToWorldCoords(transformChunkCoords);

      m_rendererPtr->Draw(meshPtr, transformWorldCoords.x,
                          transformWorldCoords.y, transformWorldCoords.z);
    }

    const auto &playerWorldCoords = m_gameStatePtr.get()->GetCamera().Position;
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

    m_windowWrapperPtr->Update();
  }

  running = false;
  dispatch.join();
}
