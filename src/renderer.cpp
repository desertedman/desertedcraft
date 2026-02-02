#include "renderer.h"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.h"

Renderer::Renderer(Camera &camera)
    : shader(Shader("./shaders/basic_vertex.glsl", "./shaders/fragment.glsl")),
      pCamera(&camera) {}

void Renderer::Draw(Block &block, int xPos, int yPos, int zPos) {
  UpdateUniforms(xPos, yPos, zPos);
  block.Draw();
}

void Renderer::UpdateUniforms(int modelX, int modelY, int modelZ) {
  // Update uniforms
  glm::mat4 projection =
      glm::perspective(glm::radians(pCamera->Zoom),
                       (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);
  glm::mat4 model = glm::mat4(1.f);
  model = glm::translate(model, glm::vec3(modelX, modelY, modelZ));
  glm::mat4 view = pCamera->GetViewMatrix();

  shader.use();
  shader.setMat4("model", model);
  shader.setMat4("view", view);
  shader.setMat4("projection", projection);
}
