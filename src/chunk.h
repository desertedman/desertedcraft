#pragma once

#include "block.h"
#include <glm/vec3.hpp>

constexpr int CHUNK_SIZE_X = 2; // horizontal
constexpr int CHUNK_SIZE_Z = 1; // depth
constexpr int CHUNK_SIZE_Y = 1; // vertical

class Chunk {
public:
  Chunk(const int xCoord = 0, const int yCoord = 0, const int zCoord = 0);
  Chunk(const glm::vec3 &coords);

  Chunk(const Chunk &other);
  Chunk &operator=(const Chunk &other);

  Chunk(Chunk &&other) noexcept;
  Chunk &operator=(Chunk &&other) noexcept;

  ~Chunk();

  void SetBlock(const BlockType blockType, const int xCoord, const int yCoord,
                const int zCoord) {
    m_pBlocks[xCoord][yCoord][zCoord].SetBlockType(blockType);
  }
  const Block &GetBlock(const int x, const int y, const int z) const;
  const Block ***const GetBlocksPtr() const;
  [[nodiscard]] const glm::vec3 GetWorldCoords() const;

private:
  void DeleteChunk();
  void CreateChunk(const int xCoord = 0, const int yCoord = 0,
                   const int zCoord = 0);

  Block ***m_pBlocks;
  glm::vec3 mWorldCoords;
};
