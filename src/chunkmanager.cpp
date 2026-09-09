#include "chunkmanager.h"
#include "block.h"
#include "chunk.h"
#include "constants.h"
#include "gamestate.h"
#include "mesher.h"
#include "oneapi/tbb/concurrent_queue.h"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <glm/ext/vector_int3.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

[[nodiscard]] glm::vec3
ChunkManager::ChunkToWorldCoords(const glm::ivec3 chunkCoords) {
  glm::ivec3 retCoords(chunkCoords.x * Constants::CHUNK_SIZE_X,
                       chunkCoords.y * Constants::CHUNK_SIZE_Y,
                       chunkCoords.z * Constants::CHUNK_SIZE_Z);

  return retCoords;
}

[[nodiscard]] glm::ivec3
ChunkManager::WorldToChunkCoords(const glm::vec3 worldCoords) {
  // Floating point division
  glm::vec3 tempCoords(worldCoords.x / Constants::CHUNK_SIZE_X,
                       worldCoords.y / Constants::CHUNK_SIZE_Y,
                       worldCoords.z / Constants::CHUNK_SIZE_Z);

  // Round down - consistent behavior for neg and pos numbers
  for (int i = 0; i < 3; i++) {
    tempCoords[i] = std::floor(tempCoords[i]);
  }

  glm::ivec3 retCoords(tempCoords.x, tempCoords.y, tempCoords.z);

  return retCoords;
}

// Constructor
ChunkManager::ChunkManager(const GameState &gamestate)
    : m_gameState(gamestate),
      m_currPlayerChunkCoords(gamestate.GetPlayerChunkCoords()) {
  m_mesherPtr = std::make_unique<MesherNaive>();

  // NOTE: Just reserved some arbitrary number
  m_chunkUnloadList.reserve(1000);
  m_chunkList.reserve(Constants::FINAL_CHUNK_DISTANCE);
  m_jobsQueuedList.reserve(Constants::FINAL_CHUNK_DISTANCE);

  m_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
}

[[nodiscard]] const Chunk *const
ChunkManager::GetChunk(const glm::ivec3 chunkCoordsPos) {
  auto iterator = m_chunkMap.find(chunkCoordsPos);
  const Chunk *const retPtr =
      iterator != m_chunkMap.end() ? iterator->second.get() : nullptr;

  return retPtr;
}

void ChunkManager::Unload(const glm::ivec3 pos) {
  auto iterator = m_chunkMap.begin();

  {
    std::scoped_lock lock(m_mutex);
    iterator = m_chunkMap.find(pos);
  }

  if (iterator == m_chunkMap.end()) {
    std::cerr << "ERROR: TRIED TO UNLOAD CHUNK; DOES NOT EXIST\n";
    return;
  }

  else {
    std::scoped_lock lock(m_mutex);
    m_chunkMap.erase(iterator);
  }
}

const std::vector<glm::ivec3> &ChunkManager::GetChunksRenderList() const {
  return m_chunkList;
}

std::unique_ptr<Chunk>
ChunkManager::GenerateChunk(const glm::ivec3 &chunkCoordsPos) {
  // Must allocate new chunk on the heap, otherwise it will be deallocated
  // immediately after allocation
  auto chunkPtr = std::make_unique<Chunk>();

  // Set height of column
  for (int x = 0; x < Constants::CHUNK_SIZE_X; x++) {
    for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++) {
      auto worldCoords = ChunkToWorldCoords(chunkCoordsPos);
      worldCoords.x += x;
      worldCoords.z += z;

      auto noise = m_noise.GetNoise((float)worldCoords.x, (float)worldCoords.z);

      // Transform noise from (-1, 1) to (0, 1)
      noise += 1;
      noise /= 2;
      noise = noise * Constants::CHUNK_SIZE_Y;

      for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++) {
        if (y > noise) {
          chunkPtr->SetBlock(BlockType::BlockType_Air, x, y, z);
        }
      }
    }
  }

  Chunk *rawChunkPtr = chunkPtr.get();

  auto meshPtr = m_mesherPtr->CreateMesh(*rawChunkPtr);
  rawChunkPtr->SetMesh(meshPtr);

  return chunkPtr;
}

// Updates Render list
void ChunkManager::Update() {
  auto newCoords = m_gameState.GetPlayerChunkCoords();
  if (m_currPlayerChunkCoords != newCoords || m_chunkList.empty()) {
    m_currPlayerChunkCoords = newCoords;

    m_chunkList.clear();
    BuildRenderList(m_currPlayerChunkCoords, m_chunkList);

    for (const auto vec : m_chunkList) {
      if (m_chunkMap.find(vec) == m_chunkMap.end()) {

        std::scoped_lock lock(m_mutex);
        if (!m_jobsQueuedList.contains(vec)) {
          m_workQueue.push(vec);
          m_jobsQueuedList.insert(vec);
          // std::cout << "MAIN: DISPATCHED JOB\n";
        }
      }

      else {
        m_jobsQueuedList.erase(vec);
        // std::cout << "MAIN: ERASED JOB\n";
      }
    }

    // if (m_jobsQueuedVector.empty()) {
    //   std::cout << "MAIN: ALL JOBS FINISHED\n";
    // }
  }

  // Upload data to GPU
  for (auto &chunkPos : m_chunkList) {
    auto it = m_chunkMap.begin();

    {
      std::scoped_lock lock(m_mutex);
      it = m_chunkMap.find(chunkPos);
    }

    if (it != m_chunkMap.end()) {
      auto meshPtr = it->second.get()->GetMeshPtr();

      if (meshPtr->isNull()) {
        meshPtr->BufferData();
      }
    }
  }

  // Unload code provided by Claude
  // NOTE: CHUNK_DISTANCE_HORIZONTAL is divided in 2 because
  // CHUNK_DISTANCE_HORIZONTAL is loaded as a square around the player. We
  // want to check the distance FROM the player, not from the opposite end of
  // the "square"
  int margin = 3;
  int maxDistance = Constants::CHUNK_DISTANCE_HORIZONTAL / 2 + margin;

  // Unload furthest chunks
  auto localCoord = m_currPlayerChunkCoords;

  for (const auto &[chunkPos, chunkPtr] : m_chunkMap) {
    auto posDiff = chunkPos - localCoord;

    // Distance from player chunk in each axis
    int dx = std::abs(posDiff.x);
    int dy = std::abs(posDiff.y);
    int dz = std::abs(posDiff.z);

    if (std::max(dx, dz) >= maxDistance) {
      m_chunkUnloadList.emplace_back(chunkPos);
    }
  }

  for (const auto chunkPos : m_chunkUnloadList) {
    // Automatically acquires lock in crit section
    Unload(chunkPos);
    std::cout << "MAIN: CHUNK UNLOADED\n";
  }

  m_chunkUnloadList.clear();
}

void ChunkManager::Dispatch(std::atomic_bool &running, int threadID) {
  glm::ivec3 vec;
  auto mainIt = m_chunkMap.begin();

  while (running) {
    try {
      m_workQueue.pop(vec);
    }

    catch (const oneapi::tbb::user_abort &) {
      // Work aborted
      break;
    }

    // std::cout << threadID << ": JOB POPPED\n";

    // Generate chunk
    auto newChunkPtr = GenerateChunk(vec);

    std::scoped_lock lock(m_mutex);
    m_chunkMap.emplace(vec, std::move(newChunkPtr));
  }
}

void ChunkManager::BuildRenderList(const glm::ivec3 playerChunkCoords,
                                   std::vector<glm::ivec3> &renderList) {
  for (int x = 0; x < Constants::CHUNK_DISTANCE_HORIZONTAL; x++)
    for (int y = 0; y < Constants::CHUNK_DISTANCE_VERTICAL; y++)
      for (int z = 0; z < Constants::CHUNK_DISTANCE_HORIZONTAL; z++) {
        // Need to offset so that player spawns in the center of these chunks
        const glm::ivec3 coords(x, y, z);

        glm::ivec3 finalChunkCoords = coords + playerChunkCoords;
        // Center the CHUNK_DISTANCE around player
        const int centerOffsetHorizontal =
            Constants::CHUNK_DISTANCE_HORIZONTAL / 2;
        const int centerOffsetVertical = Constants::CHUNK_DISTANCE_VERTICAL / 2;
        finalChunkCoords.x -= centerOffsetHorizontal;
        finalChunkCoords.z -= centerOffsetHorizontal;
        finalChunkCoords.y -= centerOffsetVertical;

        // Add chunk to the render list
        renderList.emplace_back(finalChunkCoords);
      }
}
