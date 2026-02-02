#include "renderer.h"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.h"

Renderer::Renderer(const Camera &camera)
    : shader(Shader("./shaders/basic_vertex.glsl", "./shaders/fragment.glsl")),
      mCamera(camera) {}

void Renderer::Draw(const Block &block, const int xPos, const int yPos,
                    const int zPos) const {
  UpdateUniforms(xPos, yPos, zPos);
  block.Draw();
}

void Renderer::UpdateUniforms(const int modelX, const int modelY,
                              const int modelZ) const {
  // Update uniforms
  glm::mat4 projection =
      glm::perspective(glm::radians(mCamera.Zoom),
                       (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);
  glm::mat4 model = glm::mat4(1.f);
  model = glm::translate(model, glm::vec3(modelX, modelY, modelZ));
  const glm::mat4 view = mCamera.GetViewMatrix();

  shader.use();
  shader.setMat4("model", model);
  shader.setMat4("view", view);
  shader.setMat4("projection", projection);
}
