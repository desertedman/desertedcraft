#include "block.h"

Block::Block() : isActive(true), m_blockType(BlockType::BlockType_Default) {}

BlockType Block::GetBlockType() const { return this->m_blockType; }

void Block::SetBlockType(const BlockType blockType) {
  this->m_blockType = blockType;
}
