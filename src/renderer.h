#pragma once

#include "block.h"
#include "camera.h"
#include "glad/glad.h"
#include "shader.h"
#include <GLFW/glfw3.h>

constexpr unsigned int SCR_WIDTH = 1280;
constexpr unsigned int SCR_HEIGHT = 720;

struct Color {
  float r;
  float g;
  float b;
};

class Renderer {
public:
  Renderer(const Camera &camera);
  void Draw(const Block &block, const int xPos = 0, const int yPos = 0,
            const int zPos = 0) const;

private:
  void UpdateUniforms(const int modelX, const int modelY, const int modelZ,
                      const Color &color) const;

  const Camera &mCamera;
  Shader shader;
};
