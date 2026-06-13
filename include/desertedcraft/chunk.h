#pragma once

#include "block.h"

constexpr unsigned int CHUNK_SIZE_X = 4; // horizontal
constexpr unsigned int CHUNK_SIZE_Z = 4; // depth
constexpr unsigned int CHUNK_SIZE_Y = 2; // vertical

namespace Voxel {
struct vec3 {
  vec3(const int xCoord = 0, const int yCoord = 0, const int zCoord = 0)
      : x(xCoord), y(yCoord), z(zCoord) {}

  int x{}, y{}, z{};
};
}; // namespace Voxel

class Chunk {
public:
  Chunk(const int xCoord = 0, const int yCoord = 0, const int zCoord = 0)
      : mWorldCoords(xCoord, yCoord, zCoord) {
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
  }
  ~Chunk();
  const Block &GetBlock(const int x, const int y, const int z) const;
  const Block ***const GetBlocksPtr() const {
    return const_cast<const Block ***const>(m_pBlocks);
  }
  const Voxel::vec3 &GetWorldCoords() const { return mWorldCoords; }

private:
  Block ***m_pBlocks;
  Voxel::vec3 mWorldCoords;
};
