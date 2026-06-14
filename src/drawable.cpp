#include "drawable.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

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
  const unsigned int length = vertices.size();
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, length);
}
