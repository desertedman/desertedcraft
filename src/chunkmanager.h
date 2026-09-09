#pragma once

#include "FastNoiseLite.h"
#include "chunk.h"
#include "glad/glad.h"
#include "mesher.h"
#include <GLFW/glfw3.h>
#include <atomic>
#include <cassert>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <mutex>
#include <oneapi/tbb/concurrent_queue.h>
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
constexpr int CHUNK_DISTANCE_HORIZONTAL = constexprPow(2, 4);
constexpr int CHUNK_DISTANCE_VERTICAL = 1;
// constexpr int RENDER_DISTANCE = constexprPow(2, 4);
// static_assert(RENDER_DISTANCE <= CHUNK_DISTANCE_HORIZONTAL,
//               "Render distance must be less than or equal to chunk
//               distance");

// Distance extended in all 3 axis
// NOTE: USE ONLY FOR RESERVING SPACE!! NOT FOR LOOPS OR ITERATIONS
constexpr int FINAL_CHUNK_DISTANCE = CHUNK_DISTANCE_HORIZONTAL *
                                     CHUNK_DISTANCE_HORIZONTAL *
                                     CHUNK_DISTANCE_VERTICAL;
constexpr int FINAL_RENDER_DISTANCE = FINAL_CHUNK_DISTANCE;

// For debug builds:
// 9-10 threads seems to be fastest for Linux; 4 threads for Windows
constexpr int NUM_THREADS = 9;
constexpr int NUM_WORKERS = NUM_THREADS - 1;
static_assert(NUM_THREADS >= 2, "NUM_THREADS MUST BE >= 2");

struct Job {
  int start = 0;
  int end = 0;
};

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
  [[nodiscard]] static glm::vec3
  ChunkToWorldCoords(const glm::ivec3 chunkCoords);
  [[nodiscard]] static glm::ivec3
  WorldToChunkCoords(const glm::vec3 worldCoords);

  ChunkManager(const GameState &gamestate);

  void Update();
  [[nodiscard]] const Chunk *const GetChunk(const glm::ivec3 chunkCoordsPos);
  void Unload(const glm::ivec3 pos);
  const std::vector<glm::ivec3> &GetChunksRenderList() const;
  void Dispatch(std::atomic_bool &running, int threadID);

  oneapi::tbb::concurrent_bounded_queue<Job> m_workQueue;

private:
  std::unique_ptr<Chunk> GenerateChunk(const glm::ivec3 &chunkCoordsPos);
  void BuildRenderList(const glm::ivec3 playerChunkCoords,
                       std::vector<glm::ivec3> &renderList);

  // Chunk Coords
  std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, ChunkPosHash>
      m_chunkMap;
  // Chunk Coords
  std::vector<glm::ivec3> m_chunksRenderList;
  std::vector<glm::ivec3> m_chunksUnloadList;
  const GameState &m_gameState;
  glm::ivec3 m_currPlayerChunkCoords;
  std::unique_ptr<Mesher> m_mesherPtr;
  FastNoiseLite m_noise;

private:
  // Dispatch variables
  std::vector<glm::ivec3> m_dispatchChunksRenderList;
  std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, ChunkPosHash>
      m_dispatchChunkMap;
  glm::ivec3 m_dispatchPlayerChunkCoords = glm::ivec3(0, 0, 0);
  std::atomic_bool m_isSafe;
  std::atomic_bool m_isDirty;
  std::mutex m_mutex;
};
