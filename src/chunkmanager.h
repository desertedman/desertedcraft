#pragma once

#include "FastNoiseLite.h"
#include "chunk.h"
#include "glad/glad.h"
#include "mesher.h"
#include <GLFW/glfw3.h>
#include <atomic>
#include <cassert>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_int3.hpp>
#include <memory>
#include <mutex>
#include <oneapi/tbb/concurrent_queue.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Forward declare to resolve circular dependency
class GameState;

// Hashing code provided by Claude
namespace std {
template <> struct hash<glm::ivec3> {
  size_t operator()(const glm::ivec3 &v) const noexcept {
    // pack into 64 bits and hash — avoids collisions from naive XOR
    uint64_t packed =
        (static_cast<uint64_t>(static_cast<uint32_t>(v.x)) << 32) |
        static_cast<uint32_t>(v.z);
    return std::hash<uint64_t>{}(packed);
  }
};
} // namespace std

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
  void Coordinator(std::atomic_bool &running, int threadID);

  oneapi::tbb::concurrent_bounded_queue<glm::ivec3> m_workQueue;

private:
  std::unique_ptr<Chunk> GenerateChunk(const glm::ivec3 &chunkCoordsPos);
  void BuildRenderList(const glm::ivec3 playerChunkCoords,
                       std::vector<glm::ivec3> &renderList);

  // Chunk Coords
  std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>> m_chunkMap;
  // Chunk Coords
  std::vector<glm::ivec3> m_chunkList;
  std::vector<glm::ivec3> m_chunkUnloadList;
  const GameState &m_gameState;
  glm::ivec3 m_currPlayerChunkCoords;
  std::unique_ptr<Mesher> m_mesherPtr;
  FastNoiseLite m_noise;

private:
  // Dispatch variables
  std::unordered_set<glm::ivec3> m_jobsQueuedList;
  std::mutex m_mutex;
};
