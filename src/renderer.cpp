#include "renderer.h"
#include "block.h"
#include "drawable.h"
#include "shader.h"
#include "window.h"

Renderer::Renderer(const Camera &camera)
    : mShader(Shader("./shaders/basic_vertex.glsl", "./shaders/fragment.glsl")),
      mCamera(camera) {}

// TODO: Refactor Draw function to be stateless. Create a generic "Drawable"
// class; each Drawable should store its own state (xyz coords), rather than
// being passed to this function.
// We should separate the concept of a "block" and a drawable. A block should be
// an entity in the world, and ideally should not be passed to the drawing stage
// by itself. We should dispatch a "drawable" to be created and mesh together
// all the blocks into a chunk, and then that drawable is passed to the
// renderer.
// The Draw function should ideally take in a list of drawables; such that I
// don't individually need to pass the draw function each object in the main
// rendering loop. This will make it more scalable to draw arbitrary amounts of
// objects.
void Renderer::Draw(const Block &block, const int xPos, const int yPos,
                    const int zPos) const {
  // TODO: Add proper data structures for colors; probably an array
  Color color;
  if (block.GetBlockType() == BlockType::BlockType_Dirt) {
    // Normalize color values
    color.r = 92 / 255.f;
    color.g = 70 / 255.f;
    color.b = 42 / 255.f;
  }

  else {
    // Normalize color values
    color.r = 82 / 255.f;
    color.g = 81 / 255.f;
    color.b = 81 / 255.f;
  }

  UpdateUniforms(xPos, yPos, zPos, color);
  block.Draw();
}

void Renderer::Draw(const Drawable *const drawable, const int xPos, const int yPos, const int zPos) {
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
