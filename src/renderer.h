#pragma once

#include "camera.h"
#include "glad/glad.h"
#include "mesh.h"
#include "shader.h"
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>

struct Color {
  float r;
  float g;
  float b;
};

class Renderer {
public:
  Renderer();

  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  Renderer(Renderer &&) = delete;
  Renderer &operator=(Renderer &&) = delete;

  void Draw(const Camera &camera, const Mesh *const meshPtr,
            const float xPos = 0, const float yPos = 0, const float zPos = 0);

private:
  void UpdateUniforms(const Camera &camera, const float modelX,
                      const float modelY, const float modelZ,
                      const Color &color) const;

  Shader m_shader;

  glm::vec3 m_lightPos = glm::vec3(0.f, 100.f, 0.f);
  glm::vec3 m_lightColor = glm::vec3(1.f, 1.f, 1.f);
};
