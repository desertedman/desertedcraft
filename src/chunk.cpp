#include "chunk.h"
#include "block.h"

Chunk::Chunk() {
  // First level of m_pBlocks is a Block ** pointer (Block ***)
  // Initialize first level
  m_pBlocks = new Block **[CHUNK_SIZE_X];

  for (unsigned int x = 0; x < CHUNK_SIZE_X; x++) {
    // Second level is a Block * pointer (Block **)
    m_pBlocks[x] = new Block *[CHUNK_SIZE_Y];

    for (unsigned int y = 0; y < CHUNK_SIZE_Y; y++) {
      // Third level is a Block pointer (Block *)
      m_pBlocks[x][y] = new Block[CHUNK_SIZE_Z];
    }
  }

  for (unsigned int x = 0; x < CHUNK_SIZE_X; x++)
    for (unsigned int y = 0; y < CHUNK_SIZE_Y; y++)
      for (unsigned int z = 0; z < CHUNK_SIZE_Z; z++) {
        // Y > half is dirt, else stone
        constexpr auto half = CHUNK_SIZE_Y / 2;

        if (y >= half)
          m_pBlocks[x][y][z].SetBlockType(BlockType::BlockType_Dirt);

        else
          m_pBlocks[x][y][z].SetBlockType(BlockType::BlockType_Stone);
      }

  // Test inner blocks are air
  // for (unsigned int x = 1; x < CHUNK_SIZE - 1; x++)
  //   for (unsigned int y = 1; y < CHUNK_SIZE - 1; y++)
  //     for (unsigned int z = 1; z < CHUNK_SIZE - 1; z++) {
  //       m_pBlocks[x][y][z].SetBlockType(BlockType::BlockType_Air);
  //     }
  //
  // m_pBlocks[0][0][0].SetBlockType(BlockType_Air);
}

Chunk::~Chunk() {
  // Unwind in reverse of constructor
  for (unsigned int x = 0; x < CHUNK_SIZE_X; x++) {
    for (unsigned int y = 0; y < CHUNK_SIZE_Y; y++) {
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
