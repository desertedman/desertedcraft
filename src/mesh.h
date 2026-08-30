#pragma once

#include "glm/vec3.hpp"
#include <vector>

class Mesh {
public:
  Mesh(const std::vector<glm::vec3> &inVertices);

  Mesh(const Mesh &other) = delete;
  Mesh &operator=(const Mesh &other) = delete;

  Mesh(Mesh &&other) noexcept;
  Mesh &operator=(Mesh &&other) noexcept;
  ~Mesh();

  void Draw() const;

private:
  // Need to store our own copy of vertices here, because caller (MesherNaive)
  // deallocates their own vertices vector
  std::vector<glm::vec3> m_vertices;
  unsigned int m_VAO, m_VBO, m_EBO;
};
