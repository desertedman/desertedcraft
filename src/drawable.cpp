#include "drawable.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>

DrawableMesh::DrawableMesh(const std::vector<glm::vec3> &inVertices)
    : m_vertices(inVertices) {
  glGenVertexArrays(1, &m_VAO);
  glGenBuffers(1, &m_VBO);
  glBindVertexArray(m_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3),
               m_vertices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
}

DrawableMesh::DrawableMesh(DrawableMesh &&other) noexcept
    : m_VAO(other.m_VAO), m_VBO(other.m_VBO) {
  other.m_VAO = 0;
  other.m_VBO = 0;

  std::cout << "Drawable move constructed\n";
}

DrawableMesh &DrawableMesh::operator=(DrawableMesh &&other) noexcept {
  if (this != &other) {
    // Release current objects resources first
    glDeleteBuffers(1, &m_VBO);
    glDeleteVertexArrays(1, &m_VAO);

    m_VAO = other.m_VAO;
    m_VBO = other.m_VBO;

    other.m_VAO = 0;
    other.m_VBO = 0;
  }

  std::cout << "Drawable move assigned\n";
  return *this;
}

DrawableMesh::~DrawableMesh() {
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDeleteBuffers(1, &m_VBO);
  glDeleteVertexArrays(1, &m_VAO);
}

void DrawableMesh::Draw() const {
  const int length = static_cast<int>(m_vertices.size());
  glBindVertexArray(m_VAO);
  glDrawArrays(GL_TRIANGLES, 0, length);
}
