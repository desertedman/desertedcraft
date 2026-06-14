#include "block.h"
#include "drawable.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

Block::Block() : blockType(BlockType::BlockType_Default), isActive(true) {}

void Block::Draw() const {
  drawable.Draw();
}

const BlockType Block::GetBlockType() const { return this->blockType; }

void Block::SetBlockType(const BlockType blockType) {
  this->blockType = blockType;
}
