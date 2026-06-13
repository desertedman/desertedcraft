#pragma once

#include "block.h"

constexpr unsigned int CHUNK_SIZE_X = 16; // horizontal
constexpr unsigned int CHUNK_SIZE_Z = 16; // depth
constexpr unsigned int CHUNK_SIZE_Y = 32; // vertical

class Chunk {
public:
  Chunk();
  ~Chunk();
  const Block &GetBlock(const int x, const int y, const int z) const;
  const Block ***const GetBlocksPtr() const {
    return const_cast<const Block ***const>(m_pBlocks);
  }

private:
  Block ***m_pBlocks;
};
