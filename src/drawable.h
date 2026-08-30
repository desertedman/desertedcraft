#pragma once

#include "cube.h"
#include "glm/vec3.hpp"
#include <vector>

class DrawableMesh {
public:
  DrawableMesh(const std::vector<glm::vec3> &inVertices);

  DrawableMesh(const DrawableMesh &other) = delete;
  DrawableMesh &operator=(const DrawableMesh &other) = delete;

  DrawableMesh(DrawableMesh &&other) noexcept;
  DrawableMesh &operator=(DrawableMesh &&other) noexcept;
  ~DrawableMesh();

  void Draw() const;

private:
  // Need to store our own copy of vertices here, because caller (MesherNaive)
  // deallocates their own vertices vector
  std::vector<glm::vec3> m_vertices;
  unsigned int m_VAO, m_VBO;
};
