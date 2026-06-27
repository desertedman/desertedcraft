#pragma once

#include "block.h"

constexpr int CHUNK_SIZE_X = 4; // horizontal
constexpr int CHUNK_SIZE_Z = 4; // depth
constexpr int CHUNK_SIZE_Y = 2; // vertical

class Chunk {
public:
  Chunk(const int xCoord = 0, const int yCoord = 0, const int zCoord = 0);
  Chunk(const glm::vec3 &coords);
  ~Chunk();

  // TODO: define these
  Chunk(const Chunk &other) = delete;
  Chunk(Chunk &&other) = delete;
  Chunk &operator=(const Chunk &other) = delete;
  Chunk &operator=(Chunk &&other) = delete;

  const Block &GetBlock(const int x, const int y, const int z) const;
  const Block ***const GetBlocksPtr() const;
  [[nodiscard]] const glm::vec3 GetWorldCoords() const;
  [[nodiscard]] const glm::vec3 GetChunkCoords() const;

private:
  void CreateChunk(const int xCoord = 0, const int yCoord = 0, const int zCoord = 0);
  Block ***m_pBlocks;
  glm::vec3 mWorldCoords;
};
