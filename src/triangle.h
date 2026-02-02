#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>

constexpr float triangleVerts[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
                                   0.0f,  0.0f,  0.5f, 0.0f};

class Triangle {
public:
  Triangle() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVerts), &triangleVerts,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
  }

  void Draw() {
    constexpr unsigned int length = sizeof(triangleVerts) / sizeof(float);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, length);
  }

private:
  unsigned int VAO, VBO;
};
