#include "renderer.h"
#include "drawable.h"
#include "shader.h"
#include "window.h"

Renderer::Renderer(const Camera &camera)
    : mShader(Shader("./assets/shaders/basic_vertex.glsl",
                     "./assets/shaders/fragment.glsl")),
      mCamera(camera) {}

void Renderer::Draw(const Drawable *const drawable, const int xPos,
                    const int yPos, const int zPos) {
  Color color{255, 0, 0};
  UpdateUniforms(xPos, yPos, zPos, color);
  drawable->Draw();
}

void Renderer::UpdateUniforms(const int modelX, const int modelY,
                              const int modelZ, const Color &color) const {
  // Update uniforms
  glm::mat4 projection =
      glm::perspective(glm::radians(mCamera.Zoom),
                       (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.f);
  glm::mat4 model = glm::mat4(1.f);
  model = glm::translate(model, glm::vec3(modelX, modelY, modelZ));
  const glm::mat4 view = mCamera.GetViewMatrix();

  mShader.use();
  mShader.setMat4("model", model);
  mShader.setMat4("view", view);
  mShader.setMat4("projection", projection);
  mShader.setVec3("Color", color.r, color.g, color.b);
}
