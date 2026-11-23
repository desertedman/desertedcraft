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

const Block &Chunk::GetBlock(const int x, const int y, const int z) const {
  auto index = z + y * Constants::CHUNK_SIZE_Z +
               x * Constants::CHUNK_SIZE_Z * Constants::CHUNK_SIZE_Y;
  return m_blocksArray[index];
}

Block &Chunk::GetBlock(const int x, const int y, const int z) {
  const auto &block = static_cast<const Chunk &>(*this).GetBlock(x, y, z);
  return const_cast<Block &>(block);
}

const std::array<Block, Constants::CHUNK_SIZE> Chunk::GetBlocksArray() const {
  return m_blocksArray;
}
