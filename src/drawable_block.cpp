#include "drawable_block.h"
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
