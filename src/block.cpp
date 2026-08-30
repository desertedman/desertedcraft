#include "block.h"

Block::Block() : m_blockType(BlockType::BlockType_Default), isActive(true) {}

const BlockType Block::GetBlockType() const { return this->m_blockType; }

void Block::SetBlockType(const BlockType blockType) {
  this->m_blockType = blockType;
}
