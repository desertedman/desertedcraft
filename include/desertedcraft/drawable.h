#pragma once

#include <glm/ext/vector_float3.hpp>
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

  Drawable(Drawable &&other);
  Drawable &operator=(Drawable &&other);

  virtual ~Drawable();
  virtual void Draw() const = 0;

protected:
  unsigned int VAO, VBO;
};

class DrawableBlock : Drawable {
public:
  DrawableBlock();
  void Draw() const override;

private:
};

class DrawableMesh : public Drawable {
public:
  DrawableMesh(const std::vector<glm::vec3> &inVertices);
  void Draw() const override;

private:
  // Need to store our own copy of vertices here, because caller (MesherNaive)
  // deallocates their own vertices vector
  std::vector<glm::vec3> vertices;
};
