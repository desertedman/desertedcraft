#pragma once

#include "block.h"
#include "mesh.h"
#include <array>
#include "constants.h"
#include <glm/vec3.hpp>
#include <memory>

class Chunk {
public:
  Chunk();

  Chunk(const Chunk &other) = delete;
  Chunk &operator=(const Chunk &other) = delete;

  Chunk(Chunk &&other) noexcept;
  Chunk &operator=(Chunk &&other) noexcept;

  ~Chunk() = default;

  void SetBlock(const BlockType blockType, const int xCoord, const int yCoord,
                const int zCoord);
  void SetMesh(std::unique_ptr<Mesh> &meshPtr);
  Mesh *GetMeshPtr() const { return m_meshPtr.get(); }
  const Block &GetConstBlock(const int x, const int y, const int z) const;
  Block &GetBlock(const int x, const int y, const int z);
  const std::array<Block, Constants::CHUNK_SIZE> GetBlocksArray() const;

private:
  std::unique_ptr<Mesh> m_meshPtr;
  std::array<Block, Constants::CHUNK_SIZE> m_blocksArray;
};
