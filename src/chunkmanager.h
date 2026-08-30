#pragma once

#include "chunk.h"
#include "glad/glad.h"
#include "mesher.h"
#include <GLFW/glfw3.h>
#include <cassert>
#include <memory>
#include <unordered_map>
#include <vector>

// Forward declare to resolve circular dependency
class GameState;

// constexpr power function
constexpr int constexprPow(int base, int power) {
  int value = 1;

  for (int i = 0; i < power; i++)
    value *= base;

  return value;
}

// Number of chunks to load around player. Must be a power of two in order to
// get nice cubic dimensions around the player; otherwise, the cube will be
// uneven. (ex. if player is at (0,0), then we want 2^2 = 4 nice even chunks
// surrounding the player
// TODO: Separate out chunk distance for x/z and y axis?
constexpr int CHUNK_DISTANCE = constexprPow(2, 4);
constexpr int RENDER_DISTANCE = constexprPow(2, 4);
static_assert(RENDER_DISTANCE <= CHUNK_DISTANCE,
              "Render distance must be less than or equal to chunk distance");

// Distance extended in all 3 axis
// NOTE: USE ONLY FOR RESERVING SPACE!! NOT FOR LOOPS OR ITERATIONS
constexpr int FINAL_CHUNK_DISTANCE = constexprPow(CHUNK_DISTANCE, 3);
constexpr int FINAL_RENDER_DISTANCE = constexprPow(RENDER_DISTANCE, 3);

// NOTE: ChunkCache and ChunkPosHash AI assisted by Claude
struct ChunkPosHash {
  size_t operator()(const glm::ivec3 p) const {
    // pack into 64 bits and hash — avoids collisions from naive XOR
    uint64_t packed =
        (static_cast<uint64_t>(static_cast<uint32_t>(p.x)) << 32) |
        static_cast<uint32_t>(p.z);
    return std::hash<uint64_t>{}(packed);
  }
};

class ChunkManager {
public:
  [[nodiscard]] static glm::ivec3
  ChunkToWorldCoords(const glm::ivec3 &chunkCoords);
  [[nodiscard]] static glm::ivec3
  WorldToChunkCoords(const glm::ivec3 &worldCoords);

  ChunkManager(const GameState &gamestate);

  void Update();
  const Chunk &GetChunk(const glm::ivec3 chunkCoordsPos);
  void Unload(const glm::ivec3 pos);
  const std::vector<glm::ivec3> &GetChunksRenderList() const;

private:
  // Generate new chunk and insert into map
  [[nodiscard]] Chunk *GenerateChunk(const glm::ivec3 &chunkCoordsPos);

  // Chunk Coords
  std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, ChunkPosHash>
      m_chunkMap;
  // Chunk Coords
  std::vector<glm::ivec3> m_chunksRenderList;
  const GameState &m_gameState;
  glm::ivec3 m_oldPlayerChunkCoords;
  std::unique_ptr<Mesher> m_mesher;
};
