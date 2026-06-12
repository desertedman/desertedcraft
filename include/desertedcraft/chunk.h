#pragma once

#include "block.h"

constexpr unsigned int CHUNK_SIZE = 32;

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
