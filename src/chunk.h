#pragma once

#include "block.h"
#include <array>
#include <cstddef>

constexpr std::size_t CHUNK_SIZE = 16;

class Chunk {
public:
  Chunk() {
  }

  ~Chunk() {}

private:
  std::array<std::array<std::array<Block *, CHUNK_SIZE>, CHUNK_SIZE>,
             CHUNK_SIZE>
      m_pBlocks;
};
