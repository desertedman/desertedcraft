#include "chunk.h"
#include "block.h"
#include <iostream>

// TODO: Refactor chunk generation to be faster. Every block will be
// generated, but the generation time will be slow because it has to make
// a ton of heap lookups
Chunk::Chunk(const int xCoord, const int yCoord, const int zCoord)
    : mWorldCoords(xCoord, yCoord, zCoord) {
  CreateChunk(xCoord, yCoord, zCoord);
  // std::cout << "Chunk initialized\n";
}

Chunk::Chunk(const glm::vec3 &coords)
    : Chunk(static_cast<int>(coords.x), static_cast<int>(coords.y),
            static_cast<int>(coords.z)) {}

void Chunk::DeleteChunk() {
  if (m_pBlocks) {
    // Unwind in reverse of constructor
    for (unsigned int x = 0; x < CHUNK_SIZE_X; x++) {
      for (unsigned int y = 0; y < CHUNK_SIZE_Y; y++) {
        // Delete z level
        delete[] m_pBlocks[x][y];
        m_pBlocks[x][y] = NULL;
      }

      // Delete y level
      delete[] m_pBlocks[x];
      m_pBlocks[x] = NULL;
    }

    // Delete x level
    delete[] m_pBlocks;
    m_pBlocks = NULL;
  }

  // std::cout << "Chunk deleted\n";
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
      for (unsigned int z = 0; z < CHUNK_SIZE_Z; z++) {
        if (mWorldCoords.y >= 0) {
          m_pBlocks[x][y][z].SetBlockType(BlockType_Air);
        }
      }
    }
  }
}

Chunk::~Chunk() { DeleteChunk(); }

Chunk::Chunk(const Chunk &other) : Chunk(other.mWorldCoords) {
  for (unsigned int x = 0; x < CHUNK_SIZE_X; x++) {
    for (unsigned int y = 0; y < CHUNK_SIZE_Y; y++) {
      for (unsigned int z = 0; z < CHUNK_SIZE_Z; z++) {
        const auto &otherBlock = other.m_pBlocks[x][y][z];

        m_pBlocks[x][y][z] = otherBlock;
      }
    }
  }

  std::cout << "Chunk copy constructed\n";
}

Chunk::Chunk(Chunk &&other) noexcept {
  mWorldCoords = other.mWorldCoords;
  m_pBlocks = other.m_pBlocks;
  other.m_pBlocks = NULL;

  std::cout << "Chunk move constructed\n";
}

Chunk &Chunk::operator=(const Chunk &other) {
  mWorldCoords = other.mWorldCoords;

  for (unsigned int x = 0; x < CHUNK_SIZE_X; x++) {
    for (unsigned int y = 0; y < CHUNK_SIZE_Y; y++) {
      for (unsigned int z = 0; z < CHUNK_SIZE_Z; z++) {
        const auto &otherBlock = other.m_pBlocks[x][y][z];

        m_pBlocks[x][y][z] = otherBlock;
      }
    }
  }

  std::cout << "Chunk copy assigned\n";
  return *this;
}

Chunk &Chunk::operator=(Chunk &&other) noexcept {
  if (this != &other) {
    DeleteChunk();
    mWorldCoords = other.mWorldCoords;
    m_pBlocks = other.m_pBlocks;
    other.m_pBlocks = NULL;
  }

  std::cout << "Chunk move assigned\n";
  return *this;
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
