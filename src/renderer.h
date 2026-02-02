#pragma once

#include "block.h"
#include "camera.h"
#include "glad/glad.h"
#include "shader.h"
#include <GLFW/glfw3.h>

constexpr unsigned int SCR_WIDTH = 800;
constexpr unsigned int SCR_HEIGHT = 600;

class Renderer {
public:
  Renderer(Camera &camera);
  void Draw(Block &block);

private:
  void UpdateUniforms();

  Camera *pCamera;
  Shader shader;
};
