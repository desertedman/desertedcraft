#include "renderer.h"
#include "mesh.h"
#include "shader.h"
#include "window.h"

Renderer::Renderer(const Camera &camera)
    : m_shader(Shader("./assets/shaders/basic_vertex.glsl",
                      "./assets/shaders/fragment.glsl")),
      m_camera(camera) {}

void Renderer::Draw(const Mesh *const meshPtr, const float xPos,
                    const float yPos, const float zPos) {
  Color color{1, 0, 0};
  UpdateUniforms(xPos, yPos, zPos, color);
  meshPtr->Draw();
}

void Renderer::UpdateUniforms(const float modelX, const float modelY,
                              const float modelZ, const Color &color) const {
  // Update uniforms
  glm::mat4 projection =
      glm::perspective(glm::radians(m_camera.Zoom),
                       (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.f);
  glm::mat4 model = glm::mat4(1.f);
  model = glm::translate(model, glm::vec3(modelX, modelY, modelZ));
  const glm::mat4 view = m_camera.GetViewMatrix();

  m_shader.use();
  m_shader.setMat4("model", model);
  m_shader.setMat4("view", view);
  m_shader.setMat4("projection", projection);
  m_shader.setVec3("Color", color.r, color.g, color.b);
  m_shader.setVec3("lightPos", lightPos);
  m_shader.setVec3("lightColor", lightColor);
}
