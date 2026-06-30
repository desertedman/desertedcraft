#pragma once

#include "glm/glm.hpp"
#include <vector>

constexpr float cubeVertices[] = {
    // Front (+Z)
    0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,

    0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f,

    // Back (-Z)
    1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,

    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,

    // Left (-X)
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 1.0f,

    0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 0.0f,

    // Right (+X)
    1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f,

    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f,

    // Bottom (-Y)
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f,

    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,

    // Top (+Y)
    0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f,

    0.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f
};

class Drawable {
public:
  Drawable() = default;

  // NOTE: Copying Drawables is PROBABLY not what you want....
  Drawable(const Drawable &other) = delete;
  Drawable operator=(const Drawable &other) = delete;

  Drawable(Drawable &&other) noexcept;
  Drawable &operator=(Drawable &&other) noexcept;

  // Does not need to be virtual, because child classes don't do any special allocation...
  ~Drawable();
  virtual void Draw() const = 0;

protected:
  unsigned int VAO, VBO;
};

class DrawableBlock : public Drawable {
public:
  DrawableBlock();
  void Draw() const override;

private:
};

class DrawableMesh : public Drawable {
public:
  DrawableMesh(const std::vector<glm::vec3> &inVertices);

  DrawableMesh(const DrawableMesh &other) = delete;
  DrawableMesh &operator=(const DrawableMesh &other) = delete;

  DrawableMesh(DrawableMesh &&other) noexcept = default;
  DrawableMesh &operator=(DrawableMesh &&other) noexcept = default;

  void Draw() const override;

private:
  // Need to store our own copy of vertices here, because caller (MesherNaive)
  // deallocates their own vertices vector
  std::vector<glm::vec3> vertices;
};
