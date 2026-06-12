#include "drawable_mesh.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

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
