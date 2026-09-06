#include "chunk.h"
#include "block.h"

Chunk::Chunk() : m_meshPtr(nullptr) {}

void Chunk::SetBlock(const BlockType blockType, const int xCoord,
                     const int yCoord, const int zCoord) {
  auto &block = GetBlock(xCoord, yCoord, zCoord);
  block.SetBlockType(blockType);
}

void Chunk::SetMesh(std::unique_ptr<Mesh> &meshPtr) {
  m_meshPtr = std::move(meshPtr);
}

// Chunk::Chunk(const Chunk &other) {
//   m_blocksArray = other.m_blocksArray;
//
//   // std::cout << "Chunk copy constructed\n";
// }
//
// Chunk &Chunk::operator=(const Chunk &other) {
//   m_blocksArray = other.m_blocksArray;
//
//   // std::cout << "Chunk copy assigned\n";
//   return *this;
// }

Chunk::Chunk(Chunk &&other) noexcept {
  m_blocksArray = std::move(other.m_blocksArray);
  m_meshPtr = std::move(other.m_meshPtr);

  // std::cout << "Chunk move constructed\n";
}

Chunk &Chunk::operator=(Chunk &&other) noexcept {
  if (this != &other) {
    m_blocksArray = std::move(other.m_blocksArray);
    m_meshPtr = std::move(other.m_meshPtr);
  }

  // std::cout << "Chunk move assigned\n";
  return *this;
}

const Block &Chunk::GetConstBlock(const int x, const int y, const int z) const {
  // Code provided by Claude
  auto index = z + y * CHUNK_SIZE_Z + x * CHUNK_SIZE_Z * CHUNK_SIZE_Y;
  return m_blocksArray[index];
}

Block &Chunk::GetBlock(const int x, const int y, const int z) {
  const auto &block = GetConstBlock(x, y, z);
  return const_cast<Block &>(block);
}

const std::array<Block, CHUNK_SIZE> Chunk::GetBlocksArray() const {
  return m_blocksArray;
}
