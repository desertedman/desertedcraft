#include "chunk.h"
#include "block.h"
#include <iostream>
// #include <iostream>

// TODO: Refactor chunk generation to be faster. Every block will be
// generated, but the generation time will be slow because it has to make
// a ton of heap lookups
Chunk::Chunk(const int xCoord, const int yCoord, const int zCoord)
    : m_worldCoords(xCoord, yCoord, zCoord) {
  CreateChunk(xCoord, yCoord, zCoord);
  // std::cout << "Chunk initialized\n";
}

Chunk::Chunk(const glm::ivec3 &coords) : Chunk(coords.x, coords.y, coords.z) {}

void Chunk::DeleteChunk() {
  if (m_blocksPtr) {
    // Unwind in reverse of constructor
    for (unsigned int x = 0; x < CHUNK_SIZE_X; x++) {
      for (unsigned int y = 0; y < CHUNK_SIZE_Y; y++) {
        // Delete z level
        delete[] m_blocksPtr[x][y];
        m_blocksPtr[x][y] = NULL;
      }

      // Delete y level
      delete[] m_blocksPtr[x];
      m_blocksPtr[x] = NULL;
    }

    // Delete x level
    delete[] m_blocksPtr;
    m_blocksPtr = NULL;
  }

  // std::cout << "Chunk deleted\n";
}

void Chunk::CreateChunk(const int xCoord, const int yCoord, const int zCoord) {
  // First level of m_pBlocks is a Block ** pointer (Block ***)
  // Initialize first level
  m_blocksPtr = new Block **[CHUNK_SIZE_X];
  m_meshPtr = nullptr;

  for (unsigned int x = 0; x < CHUNK_SIZE_X; x++) {
    // Second level is a Block * pointer (Block **)
    m_blocksPtr[x] = new Block *[CHUNK_SIZE_Y];

    for (unsigned int y = 0; y < CHUNK_SIZE_Y; y++) {
      // Third level is a Block pointer (Block *)
      m_blocksPtr[x][y] = new Block[CHUNK_SIZE_Z];
    }
  }
}

Chunk::~Chunk() { DeleteChunk(); }
void Chunk::SetBlock(const BlockType blockType, const int xCoord,
                     const int yCoord, const int zCoord) {
  m_blocksPtr[xCoord][yCoord][zCoord].SetBlockType(blockType);
}

void Chunk::SetMesh(std::unique_ptr<Mesh> &meshPtr) {
  m_meshPtr = std::move(meshPtr);
}

Chunk::Chunk(const Chunk &other) : Chunk(other.m_worldCoords) {
  for (unsigned int x = 0; x < CHUNK_SIZE_X; x++) {
    for (unsigned int y = 0; y < CHUNK_SIZE_Y; y++) {
      for (unsigned int z = 0; z < CHUNK_SIZE_Z; z++) {
        const auto &otherBlock = other.m_blocksPtr[x][y][z];

        m_blocksPtr[x][y][z] = otherBlock;
      }
    }
  }

  // std::cout << "Chunk copy constructed\n";
}

Chunk::Chunk(Chunk &&other) noexcept {
  m_worldCoords = other.m_worldCoords;
  m_blocksPtr = other.m_blocksPtr;
  other.m_blocksPtr = NULL;

  // std::cout << "Chunk move constructed\n";
}

Chunk &Chunk::operator=(const Chunk &other) {
  m_worldCoords = other.m_worldCoords;

  for (unsigned int x = 0; x < CHUNK_SIZE_X; x++) {
    for (unsigned int y = 0; y < CHUNK_SIZE_Y; y++) {
      for (unsigned int z = 0; z < CHUNK_SIZE_Z; z++) {
        const auto &otherBlock = other.m_blocksPtr[x][y][z];

        m_blocksPtr[x][y][z] = otherBlock;
      }
    }
  }

  // std::cout << "Chunk copy assigned\n";
  return *this;
}

Chunk &Chunk::operator=(Chunk &&other) noexcept {
  if (this != &other) {
    DeleteChunk();
    m_worldCoords = other.m_worldCoords;
    m_blocksPtr = other.m_blocksPtr;
    other.m_blocksPtr = NULL;
  }

  // std::cout << "Chunk move assigned\n";
  return *this;
}

const Block &Chunk::GetBlock(const int x, const int y, const int z) const {
  return m_blocksPtr[x][y][z];
}

const Block ***const Chunk::GetBlocksPtr() const {
  return const_cast<const Block ***const>(m_blocksPtr);
}

const glm::ivec3 Chunk::GetWorldCoords() const { return m_worldCoords; }
