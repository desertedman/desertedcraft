#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "drawable.h"

class DrawableMesh : public Drawable {
public:
  DrawableMesh(const std::vector<glm::vec3> &inVertices);
  virtual void Draw() const override;

private:
  std::vector<glm::vec3> vertices;
};
