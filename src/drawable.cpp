#include "drawable.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>

Drawable::Drawable(Drawable &&other) noexcept : VAO(other.VAO), VBO(other.VBO) {
  other.VAO = 0;
  other.VBO = 0;

  std::cout << "Drawable move constructed\n";
}

Drawable &Drawable::operator=(Drawable &&other) noexcept {
  if (this != &other) {
    // Release current objects resources first
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    VAO = other.VAO;
    VBO = other.VBO;

    other.VAO = 0;
    other.VBO = 0;
  }

  std::cout << "Drawable move assigned\n";
  return *this;
}

Drawable::~Drawable() {
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDeleteBuffers(1, &VBO);
  glDeleteVertexArrays(1, &VAO);

  // std::cerr << "Drawable destroyed\n";
}

DrawableBlock::DrawableBlock() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices,
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
}

void DrawableBlock::Draw() const {
  constexpr unsigned int length = sizeof(cubeVertices) / sizeof(float);
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, length);
}

DrawableMesh::DrawableMesh(const std::vector<glm::vec3> &inVertices)
    : vertices(inVertices) {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
               vertices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
}

void DrawableMesh::Draw() const {
  const int length = static_cast<int>(vertices.size());
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, length);
}
