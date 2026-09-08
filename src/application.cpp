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
#include <algorithm>
#include <cassert>
#include <chrono>
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
  glfwSwapInterval(0); // Disable vsync
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

void Application::Run() {
  auto &chunkManager = m_gameStatePtr->chunkManager;
  const auto &renderList = chunkManager.GetChunksRenderList();

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // Spawn camera at half height
  auto &cameraPos = m_gameStatePtr->GetCamera().Position;
  cameraPos.y = float(CHUNK_SIZE_Y) / 2;

  std::atomic_bool running = true;

  std::vector<std::thread> workers;
  workers.reserve(NUM_WORKERS);
  for (int i = 0; i < NUM_WORKERS; i++) {
    workers.emplace_back(&ChunkManager::Dispatch, &chunkManager,
                         std::ref(running), i + 1);
  }

  auto startTime = std::chrono::steady_clock::now();
  std::vector<int> doneVec(FINAL_CHUNK_DISTANCE);
  bool reported = false;

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

    for (int i = 0; i < renderList.size(); i++) {
      const auto &transformChunkCoords = renderList[i];
      const auto chunkPtr = chunkManager.GetChunk(transformChunkCoords);
      if (chunkPtr) {
        const auto *meshPtr = chunkPtr->GetMeshPtr();

        const auto transformWorldCoords =
            ChunkManager::ChunkToWorldCoords(transformChunkCoords);

        m_rendererPtr->Draw(meshPtr, transformWorldCoords.x,
                            transformWorldCoords.y, transformWorldCoords.z);

        doneVec[i] = 1;
      }

      else {
        doneVec[i] = 0;
      }
    }

    // Report how long it took to generate all chunks
    if (std::find(doneVec.begin(), doneVec.end(), 0) == doneVec.end() &&
        !reported) {
      auto endTime = std::chrono::steady_clock::now();
      auto elapsed =
          std::chrono::duration<double, std::milli>(endTime - startTime);

      std::cout << "TIME: " << elapsed.count() << " ms\n";

      startTime = std::chrono::steady_clock::now();

      reported = true;
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

  chunkManager.m_workQueue.abort();

  for (auto &worker : workers) {
    worker.join();
  }
}
