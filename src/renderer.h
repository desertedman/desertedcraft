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
  void Draw(Block &block, int xPos = 0, int yPos = 0, int zPos = 0);

private:
  void UpdateUniforms(int modelX, int modelY, int modelZ);

  Camera *pCamera;
  Shader shader;
};
