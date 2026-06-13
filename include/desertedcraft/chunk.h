#pragma once

#include "block.h"

constexpr unsigned int CHUNK_SIZE_X = 4; // horizontal
constexpr unsigned int CHUNK_SIZE_Z = 4; // depth
constexpr unsigned int CHUNK_SIZE_Y = 2; // vertical

namespace Voxel {
struct vec3 {
  vec3(const int xCoord = 0, const int yCoord = 0, const int zCoord = 0)
      : x(xCoord), y(yCoord), z(zCoord) {}

  int x, y, z;
};
}; // namespace Voxel

class Chunk {
public:
  Chunk(const int xCoord = 0, const int yCoord = 0, const int zCoord = 0);
  ~Chunk();
  const Block &GetBlock(const int x, const int y, const int z) const;
  const Block ***const GetBlocksPtr() const;
  const Voxel::vec3 &GetWorldCoords() const;

private:
  Block ***m_pBlocks;
  Voxel::vec3 mWorldCoords;
};
