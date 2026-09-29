#pragma once

#include <glm/ext/vector_float3.hpp>
#include <vector>

class Mesh {
public:
  Mesh(const std::vector<glm::vec3> &inVertices);

  Mesh(const Mesh &other) = delete;
  Mesh &operator=(const Mesh &other) = delete;

  Mesh(Mesh &&other) noexcept;
  Mesh &operator=(Mesh &&other) noexcept;
  ~Mesh();

  // TODO: Change to lazy upload and bind buffers at draw time
  void Draw() const;
  void BufferData();
  bool IsNull();

private:
  // Need to store our own copy of vertices here, because caller (MesherNaive)
  // deallocates their own vertices vector
  std::vector<glm::vec3> m_vertices;
  unsigned int m_VAO, m_VBO;
  [[maybe_unused]] unsigned int m_EBO;

  bool m_isNull;
};
