#pragma once

#include "gamestate.h"
#include "renderer.h"
#include "window.h"
#include <memory>

class Application {
public:
  Application();
  Application(const Application &other) = delete;
  Application(const Application &&other) = delete;
  Application &operator=(const Application &other) = delete;
  Application &operator=(const Application &&other) = delete;
  ~Application();

  void Run();

private:
  std::unique_ptr<Renderer> m_rendererPtr;
  std::unique_ptr<GameState> m_gameStatePtr;
  std::unique_ptr<Window> m_windowWrapperPtr;
};
