#include "chunk.h"

Chunk::Chunk() {
  // First level of m_pBlocks is a Block ** pointer (Block ***)
  // Initialize first level
  m_pBlocks = new Block **[CHUNK_SIZE];

  for (unsigned int x = 0; x < CHUNK_SIZE; x++) {
    // Second level is a Block * pointer (Block **)
    m_pBlocks[x] = new Block *[CHUNK_SIZE];

    for (unsigned int y = 0; y < CHUNK_SIZE; y++) {
      // Third level is a Block pointer (Block *)
      m_pBlocks[x][y] = new Block[CHUNK_SIZE];
    }
  }

  // Y > half is dirt, else stone
  for (unsigned int x = 0; x < CHUNK_SIZE; x++)
    for (unsigned int y = 0; y < CHUNK_SIZE; y++)
      for (unsigned int z = 0; z < CHUNK_SIZE; z++) {
        constexpr auto half = CHUNK_SIZE / 2;

        if (y >= half)
          m_pBlocks[x][y][z].SetBlockType(BlockType::BlockType_Dirt);

        else
          m_pBlocks[x][y][z].SetBlockType(BlockType::BlockType_Stone);
      }
}

Chunk::~Chunk() {
  // Unwind in reverse of constructor
  for (unsigned int x = 0; x < CHUNK_SIZE; x++) {
    for (unsigned int y = 0; y < CHUNK_SIZE; y++) {
      // Delete z level
      delete[] m_pBlocks[x][y];
    }

    // Delete y level
    delete[] m_pBlocks[x];
  }

  // Delete x level
  delete[] m_pBlocks;
}

const Block &Chunk::GetBlock(const int x, const int y, const int z) const {
  return m_pBlocks[x][y][z];
}
