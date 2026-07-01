#include "block.h"

Block::Block() : blockType(BlockType::BlockType_Default), isActive(true) {}

const BlockType Block::GetBlockType() const { return this->blockType; }

void Block::SetBlockType(const BlockType blockType) {
  this->blockType = blockType;
}
