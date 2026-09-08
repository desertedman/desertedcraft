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
  Renderer(const Camera &camera);
  Renderer(const Renderer &other) = delete;
  Renderer(const Renderer &&other) = delete;
  Renderer &operator=(const Renderer &other) = delete;
  Renderer &operator=(const Renderer &&other) = delete;

  void Draw(const Mesh *const meshPtr, const float xPos = 0,
            const float yPos = 0, const float zPos = 0);

private:
  void UpdateUniforms(const float modelX, const float modelY,
                      const float modelZ, const Color &color) const;

  const Camera &m_camera;
  Shader m_shader;

  glm::vec3 lightPos = glm::vec3(0.f, 100.f, 0.f);
  glm::vec3 lightColor = glm::vec3(1.f, 1.f, 1.f);
};
