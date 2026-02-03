#include "block.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

Block::Block() : blockType(BlockType::BlockType_Default), isActive(true) {
  glGenVertexArrays(1, &blockVAO);
  glGenBuffers(1, &blockVBO);
  glBindVertexArray(blockVAO);
  glBindBuffer(GL_ARRAY_BUFFER, blockVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(blockVerts), &blockVerts,
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
}

void Block::Draw() const {
  constexpr unsigned int length = sizeof(blockVerts) / sizeof(float);
  glBindVertexArray(blockVAO);
  glDrawArrays(GL_TRIANGLES, 0, length);
}
