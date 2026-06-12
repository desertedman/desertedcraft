#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float3.hpp>
#include <vector>

#include "drawable.h"

class DrawableMesh : public Drawable {
public:
  DrawableMesh(const std::vector<glm::vec3> &inVertices)
      : vertices(inVertices) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
                 vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          (void *)0);
  }

  virtual void Draw() const override {
    const unsigned int length = vertices.size();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, length);
  }

private:
  std::vector<glm::vec3> vertices;
};
