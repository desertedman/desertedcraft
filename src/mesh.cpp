#include "mesh.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

// TODO: Move all this binding/upload code to lazy upload at Draw time!
Mesh::Mesh(const std::vector<glm::vec3> &inVertices)
    : m_vertices(inVertices), m_isNull(true) {}

// TODO: Add checks for null before stealing resources
Mesh::Mesh(Mesh &&other) noexcept
    : m_VAO(other.m_VAO), m_VBO(other.m_VBO), m_isNull(other.m_isNull) {
  other.m_VAO = 0;
  other.m_VBO = 0;
  m_vertices = std::move(other.m_vertices);
  other.m_isNull = true;

  // std::cout << "Mesh move constructed\n";
}

Mesh &Mesh::operator=(Mesh &&other) noexcept {
  if (this != &other) {
    // Release current objects resources first
    glDeleteBuffers(1, &m_VBO);
    glDeleteVertexArrays(1, &m_VAO);
    m_vertices.clear();

    m_VAO = other.m_VAO;
    m_VBO = other.m_VBO;
    m_isNull = other.m_isNull;

    other.m_VAO = 0;
    other.m_VBO = 0;
    other.m_isNull = true;
    m_vertices = std::move(other.m_vertices);
  }

  // std::cout << "Mesh move assigned\n";
  return *this;
}

Mesh::~Mesh() {
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDeleteBuffers(1, &m_VBO);
  glDeleteVertexArrays(1, &m_VAO);
}

void Mesh::Draw() const {
  const int length = static_cast<int>(m_vertices.size());
  glBindVertexArray(m_VAO);
  glDrawArrays(GL_TRIANGLES, 0, length);
}

void Mesh::BufferData() {
  glGenVertexArrays(1, &m_VAO);
  glGenBuffers(1, &m_VBO);
  glBindVertexArray(m_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3),
               m_vertices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

  m_isNull = false;
}

bool Mesh::isNull() { return m_isNull; }
