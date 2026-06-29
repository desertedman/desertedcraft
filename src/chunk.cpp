#include "chunk.h"
#include "block.h"
#include <iostream>

// TODO: Refactor chunk generation to be faster. Every block will be
// generated, but the generation time will be slow because it has to make
// a ton of heap lookups
Chunk::Chunk(const int xCoord, const int yCoord, const int zCoord)
    : mWorldCoords(xCoord, yCoord, zCoord) {
  CreateChunk(xCoord, yCoord, zCoord);
}

Chunk::Chunk(const glm::vec3 &coords) : mWorldCoords(coords) {
  CreateChunk(coords.x, coords.y, coords.z);
}

void Chunk::CreateChunk(const int xCoord, const int yCoord, const int zCoord) {
  // First level of m_pBlocks is a Block ** pointer (Block ***)
  // Initialize first level
  m_pBlocks = new Block **[CHUNK_SIZE_X];

  for (unsigned int x = 0; x < CHUNK_SIZE_X; x++) {
    // Second level is a Block * pointer (Block **)
    m_pBlocks[x] = new Block *[CHUNK_SIZE_Y];

    for (unsigned int y = 0; y < CHUNK_SIZE_Y; y++) {
      // Third level is a Block pointer (Block *)
      m_pBlocks[x][y] = new Block[CHUNK_SIZE_Z];

      // DEBUG: Set blocks higher than y = 0 to air
      // for (unsigned int z = 0; z < CHUNK_SIZE_Z; z++) {
      //   if (mWorldCoords.y >= 0) {
      //     m_pBlocks[x][y][z].SetBlockType(BlockType_Air);
      //   }
      // }
    }
  }

  std::cout << "Chunk initialized\n";
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

  std::cout << "Chunk deleted\n";
}

const Block &Chunk::GetBlock(const int x, const int y, const int z) const {
  return m_pBlocks[x][y][z];
}

const Block ***const Chunk::GetBlocksPtr() const {
  return const_cast<const Block ***const>(m_pBlocks);
}

// NOTE: Normally this should never return non-integer coordinates. Best to
// check, though!
const glm::vec3 Chunk::GetWorldCoords() const { return mWorldCoords; }
