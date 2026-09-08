#include "chunkmanager.h"
#include "block.h"
#include "chunk.h"
#include "gamestate.h"
#include "mesher.h"
#include "oneapi/tbb/concurrent_queue.h"
#include <atomic>
#include <chrono>
#include <cmath>
#include <glm/ext/vector_int3.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

[[nodiscard]] glm::vec3
ChunkManager::ChunkToWorldCoords(const glm::ivec3 chunkCoords) {
  glm::ivec3 retCoords(chunkCoords.x * CHUNK_SIZE_X,
                       chunkCoords.y * CHUNK_SIZE_Y,
                       chunkCoords.z * CHUNK_SIZE_Z);

  return retCoords;
}

[[nodiscard]] glm::ivec3
ChunkManager::WorldToChunkCoords(const glm::vec3 worldCoords) {
  // Floating point division
  glm::vec3 tempCoords(worldCoords.x / CHUNK_SIZE_X,
                       worldCoords.y / CHUNK_SIZE_Y,
                       worldCoords.z / CHUNK_SIZE_Z);

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
      m_currPlayerChunkCoords(gamestate.GetPlayerChunkCoords()),
      m_isDirty(true), m_isSafe(false) {
  m_mesherPtr = std::make_unique<MesherNaive>();

  // NOTE: Just reserved some arbitrary number
  m_chunksUnloadList.reserve(1000);
  m_chunksRenderList.reserve(FINAL_CHUNK_DISTANCE);

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
  return m_chunksRenderList;
}

std::unique_ptr<Chunk>
ChunkManager::GenerateChunk(const glm::ivec3 &chunkCoordsPos) {
  // Must allocate new chunk on the heap, otherwise it will be deallocated
  // immediately after allocation
  auto chunkPtr = std::make_unique<Chunk>();

  // Set height of column
  // TODO: TRANSFORM FROM LOCAL CHUNK COORDS TO WORLD COORDS!!!
  for (int x = 0; x < CHUNK_SIZE_X; x++) {
    for (int z = 0; z < CHUNK_SIZE_Z; z++) {
      auto worldCoords = ChunkToWorldCoords(chunkCoordsPos);
      worldCoords.x += x;
      worldCoords.z += z;

      auto noise = m_noise.GetNoise((float)worldCoords.x, (float)worldCoords.z);

      // Transform noise from (-1, 1) to (0, 1)
      noise += 1;
      noise /= 2;
      noise = noise * CHUNK_SIZE_Y;

      for (int y = 0; y < CHUNK_SIZE_Y; y++) {
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
void ChunkManager::Update(std::atomic_bool &running) {
  auto newCoords = m_gameState.GetPlayerChunkCoords();
  if (m_currPlayerChunkCoords != newCoords || m_chunksRenderList.empty()) {
    m_currPlayerChunkCoords = newCoords;

    {
      std::scoped_lock lock(m_mutex);
      m_chunksRenderList.clear();
      BuildRenderList(m_currPlayerChunkCoords, m_chunksRenderList);
    }

    // Calculate work division
    const int size = m_chunksRenderList.size();
    for (int i = 0; i < NUM_WORKERS; i++) {

      const int offset = size / NUM_WORKERS;

      const int start = i / size + i * offset;
      const int end = size / (i + 1) + i * offset;

      const Job job{.start = start, .end = end};

      m_workQueue.push(job);
      std::cout << "MAIN: JOB PUSHED\n";
      std::cout << "start: " << start << "\n";
      std::cout << "end: " << end << "\n";
      std::cout << "offset: " << offset << "\n";
      std::cout << "size: " << size << "\n";
    }
  }

  // Upload data to GPU
  for (auto &chunkPos : m_chunksRenderList) {
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
  int maxDistance = CHUNK_DISTANCE_HORIZONTAL / 2 + margin;

  // Unload furthest chunks
  auto localCoord = m_currPlayerChunkCoords;

  for (const auto &[chunkPos, chunkPtr] : m_chunkMap) {
    auto posDiff = chunkPos - localCoord;

    // Distance from player chunk in each axis
    int dx = std::abs(posDiff.x);
    int dy = std::abs(posDiff.y);
    int dz = std::abs(posDiff.z);

    if (std::max(dx, dz) >= maxDistance) {
      m_chunksUnloadList.emplace_back(chunkPos);
    }
  }

  for (const auto chunkPos : m_chunksUnloadList) {
    // Automatically acquires lock in crit section
    Unload(chunkPos);
    std::cout << "MAIN: CHUNK UNLOADED\n";
  }

  m_chunksUnloadList.clear();
}

// TODO: Add unloading logic
void ChunkManager::Dispatch(std::atomic_bool &running, int threadID) {
  // Timing code provided by Claude
  const int TARGET_HZ = 20;
  const auto TICK_DUR = std::chrono::microseconds(1000000 / TARGET_HZ);

  Job job;

  auto mainIt = m_chunkMap.begin();
  std::vector<glm::ivec3> localRenderList;

  while (running) {
    auto startTime = std::chrono::steady_clock::now();

    try {
      m_workQueue.pop(job);
      std::cout << threadID << ": JOB POPPED\n";
    }

    catch (const oneapi::tbb::user_abort &) {
      // Work aborted
      continue;
    }

    {
      std::scoped_lock lock(m_mutex);
      localRenderList = m_chunksRenderList;
    }

    // Generate each chunk and corresponding mesh
    for (int i = job.start; i < job.end; i++) {
      if (!running) {
        break;
      }

      const auto vec = localRenderList[i];

      // Check if we have the chunk already cached
      {
        std::scoped_lock lock(m_mutex);
        mainIt = m_chunkMap.find(vec);
      }

      // Cached chunk not found - generate new chunk
      if (mainIt == m_chunkMap.end()) {
        auto newChunkPtr = GenerateChunk(vec);

        {
          std::scoped_lock lock(m_mutex);
          m_chunkMap.emplace(vec, std::move(newChunkPtr));
        }

        std::cout << threadID << ": GENERATED NEW CHUNK; MOVED TO MAIN MAP\n";
      }

      else
        std::cout << threadID << ": CACHED CHUNK FOUND IN MAIN MAP\n";
    }

    std::cout << threadID << ": ALL CHUNKS GENERATED\n";

    auto endTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime);

    // Finished work early
    if (elapsed < TICK_DUR)
      std::this_thread::sleep_for(TICK_DUR - elapsed);
  }
}

void ChunkManager::BuildRenderList(const glm::ivec3 playerChunkCoords,
                                   std::vector<glm::ivec3> &renderList) {
  for (int x = 0; x < CHUNK_DISTANCE_HORIZONTAL; x++)
    for (int y = 0; y < CHUNK_DISTANCE_VERTICAL; y++)
      for (int z = 0; z < CHUNK_DISTANCE_HORIZONTAL; z++) {
        // Need to offset so that player spawns in the center of these chunks
        const glm::ivec3 coords(x, y, z);

        glm::ivec3 finalChunkCoords = coords + playerChunkCoords;
        // Center the CHUNK_DISTANCE around player
        const int centerOffsetHorizontal = CHUNK_DISTANCE_HORIZONTAL / 2;
        const int centerOffsetVertical = CHUNK_DISTANCE_VERTICAL / 2;
        finalChunkCoords.x -= centerOffsetHorizontal;
        finalChunkCoords.z -= centerOffsetHorizontal;
        finalChunkCoords.y -= centerOffsetVertical;

        // Add chunk to the render list
        renderList.emplace_back(finalChunkCoords);
      }
}
