#pragma once

#include "block.h"

constexpr unsigned int CHUNK_SIZE = 4;

class Chunk {
public:
  Chunk();
  ~Chunk();
  const Block &GetBlock(const int x, const int y, const int z) const;

private:
  Block ***m_pBlocks;
};
