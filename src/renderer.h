#pragma once

#include "block.h"
#include "camera.h"
#include "drawable.h"
#include "glad/glad.h"
#include "shader.h"
#include <GLFW/glfw3.h>

struct Color {
  float r;
  float g;
  float b;
};

class Renderer {
public:
  Renderer(const Camera &camera);
  Renderer(const Renderer &other) = delete;
  Renderer(const Renderer &&other) = delete;
  Renderer &operator=(const Renderer &other) = delete;
  Renderer &operator=(const Renderer &&other) = delete;

  void Draw(const Block &block, const int xPos = 0, const int yPos = 0,
            const int zPos = 0) const;
  void Draw(const Drawable *const drawable, const int xPos = 0,
            const int yPos = 0, const int zPos = 0);

private:
  void UpdateUniforms(const int modelX, const int modelY, const int modelZ,
                      const Color &color) const;

  const Camera &mCamera;
  Shader mShader;
};
