#include "chunkmanager.h"
#include "block.h"
#include "chunk.h"
#include "gamestate.h"
#include "mesher.h"
#include <atomic>
#include <chrono>
#include <cmath>
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

ChunkManager::ChunkManager(const GameState &gamestate)
    : m_gameState(gamestate),
      m_currPlayerChunkCoords(gamestate.GetPlayerChunkCoords()),
      m_isDirty(true), m_isSafe(false) {
  m_mesherPtr = std::make_unique<MesherNaive>();
  // NOTE: Just reserved some arbitrary number
  m_chunksUnloadList.reserve(1000);

  m_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
}

// Updates Render list
void ChunkManager::Update() {
  // TODO: The only way I can make this perform reasonably better is if I had a
  // way to represent and skip over null chunks.
  if (m_isDirty && m_isSafe) {
    {
      std::scoped_lock(mutex);
      m_currPlayerChunkCoords = m_dispatchPlayerChunkCoords;
      m_chunksRenderList = m_dispatchChunksRenderList;

      for (auto vec : m_chunksRenderList) {
        const auto iterator = m_chunkMap.find(vec);

        // Move chunk from dispatch map to main map
        if (iterator == m_chunkMap.end()) {
          auto &dispatchChunk = m_dispatchChunkMap.find(vec)->second;
          m_chunkMap.emplace(vec, std::move(dispatchChunk));
        }
      }

      m_dispatchChunkMap.clear();
    }

    m_isDirty = false;
    std::cout << "MAIN: \t\tMOVED TO MAIN THREAD\n";
  }

  // Upload data to GPU
  for (auto &chunkPos : m_chunksRenderList) {
    auto it = m_chunkMap.find(chunkPos);
    auto meshPtr = it->second.get()->GetMeshPtr();

    if (meshPtr->isNull()) {
      meshPtr->BufferData();
    }
  }

  // Unload code provided by Claude
  // NOTE: CHUNK_DISTANCE_HORIZONTAL is divided in 2 because
  // CHUNK_DISTANCE_HORIZONTAL is loaded as a square around the player. We want
  // to check the distance FROM the player, not from the opposite end of the
  // "square"
  int margin = 3;
  int maxDistance = CHUNK_DISTANCE_HORIZONTAL / 2 + margin;

  // Unload furthest chunks
  // TODO: This causes HEAVY stalling while chunks have to be force generated.
  // Investigate why!

  glm::ivec3 localCoord = glm::ivec3(0, 0, 0);
  {
    std::scoped_lock lock(m_mutex);
    localCoord = m_currPlayerChunkCoords;
  }

  for (const auto &[chunkPos, chunkPtr] : m_chunkMap) {
    // Worker thread consistently updates player coords, while main thread
    // sometimes misses updates to chunk list. This explains why we sometimes
    // unload chunks that we need to force regenerate moments later. The main
    // thread render list is severely outdated in relation to the player coords.
    // This unload function collects chunks relative to the current (up-to-date)
    // player coords, while the main thread renders the chunk list relative to
    // outdated player coords.
    auto posDiff = chunkPos - localCoord;

    // Distance from player chunk in each axis
    int dx = std::abs(posDiff.x);
    int dy = std::abs(posDiff.y);
    int dz = std::abs(posDiff.z);

    if (std::max(dx, dz) >= maxDistance) {
      m_chunksUnloadList.emplace_back(chunkPos);
    }
  }

  {
    std::scoped_lock lock(m_mutex);
    for (const auto chunkPos : m_chunksUnloadList) {
      Unload(chunkPos);
      std::cout << "MAIN: CHUNK UNLOADED\n";
    }
  }

  m_chunksUnloadList.clear();
}

[[nodiscard]] const Chunk &
ChunkManager::GetChunk(const glm::ivec3 chunkCoordsPos) {
  auto iterator = m_chunkMap.find(chunkCoordsPos);
  const Chunk *retPtr;

  // Cached item found
  if (iterator != m_chunkMap.end()) {
    retPtr = iterator->second.get();
  }

  // Cached item not found - generate new item
  else {
    // NOTE: Normally, this case should NEVER be hit. However, because the main
    // thread can sometimes miss the worker thread updating the chunk list, the
    // main thread tries to render a chunk, but the main thread has already ran
    // the unload function on some specific chunk. Because the main thread chunk
    // list is out of date, we're force regenerating chunks that we just
    // unloaded. I suspect this is what is causing our huge stalls.
    std::cerr << "ERROR: FORCE GENERATING CHUNK!!!\n";
    auto unique = GenerateChunk(chunkCoordsPos);
    retPtr = unique.get();
    m_chunkMap.emplace(chunkCoordsPos, std::move(unique));
  }

  return *retPtr;
}

[[nodiscard]] const Chunk &ChunkManager::GetChunk(
    const glm::ivec3 chunkCoordsPos,
    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, ChunkPosHash>
        &chunkCache) {
  auto iterator = chunkCache.find(chunkCoordsPos);
  const Chunk *retPtr;

  // Cached item found
  if (iterator != m_chunkMap.end()) {
    retPtr = iterator->second.get();
  }

  // NOTE: If we run this function on the main thread, ideally this case should
  // NEVER be hit.

  // Cached item not found - generate new item
  else {
    auto unique = GenerateChunk(chunkCoordsPos);
    retPtr = unique.get();
    chunkCache.emplace(chunkCoordsPos, std::move(unique));
  }

  return *retPtr;
}

void ChunkManager::Unload(const glm::ivec3 pos) {
  auto iterator = m_chunkMap.find(pos);
  if (iterator == m_chunkMap.end()) {
    std::cerr << "ERROR: TRIED TO UNLOAD CHUNK; DOES NOT EXIST\n";
    return;
  }

  else {
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
  auto chunkPtr = std::make_unique<Chunk>(chunkCoordsPos.x, chunkCoordsPos.y,
                                          chunkCoordsPos.z);

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

  auto meshPtr = m_mesherPtr->CreateMesh(rawChunkPtr->GetBlocksPtr());
  rawChunkPtr->SetMesh(meshPtr);

  return chunkPtr;
}

// TODO: Add unloading logic
void ChunkManager::Dispatch(std::atomic_bool &running) {
  const int TARGET_HZ = 20;
  const auto TICK_DUR = std::chrono::microseconds(1000000 / TARGET_HZ);

  while (running) {
    auto startTime = std::chrono::steady_clock::now();

    {
      std::scoped_lock lock(m_mutex);
      m_dispatchPlayerChunkCoords = m_gameState.GetPlayerChunkCoords();
    }

    // If render list is empty, need to populate it
    if (m_dispatchPlayerChunkCoords != m_currPlayerChunkCoords ||
        m_dispatchChunksRenderList.empty()) {
      std::cout << "DISPATCH: RENDER LIST DIRTY\n";

      {
        std::scoped_lock lock(m_mutex);

        m_isDirty = true;
        m_isSafe = false;

        // Build new render list
        m_dispatchChunksRenderList.clear();
        BuildRenderList(m_dispatchPlayerChunkCoords,
                        m_dispatchChunksRenderList);
      }
      std::cout << "DISPATCH: RENDER LIST POPULATED\n";

      // Generate each chunk and corresponding mesh
      for (const auto vec : m_dispatchChunksRenderList) {
        if (!running) {
          break;
        }

        // Also check if the dispatch map already has the chunk cached
        auto mainIt = m_chunkMap.begin();
        auto dispatchIt = m_dispatchChunkMap.begin();

        {
          std::scoped_lock lock(m_mutex);
          mainIt = m_chunkMap.find(vec);
          dispatchIt = m_dispatchChunkMap.find(vec);
        }

        // Cached chunk not found - generate new chunk
        if (mainIt == m_chunkMap.end() &&
            dispatchIt == m_dispatchChunkMap.end()) {
          auto newChunkPtr = GenerateChunk(vec);

          {
            std::scoped_lock lock(m_mutex);
            m_dispatchChunkMap.emplace(vec, std::move(newChunkPtr));
          }
          std::cout << "DISPATCH: GENERATED NEW CHUNK\n";
        }

        // Cached chunk found in main map
        else {
          if (mainIt != m_chunkMap.end())
            std::cout << "DISPATCH: CACHED CHUNK FOUND IN MAIN MAP\n";

          else
            std::cout << "DISPATCH: CACHED CHUNK FOUND IN DISPATCH MAP\n";

          continue;
        }
      }

      std::cout << "DISPATCH: LOADED ALL CHUNKS\n";
      std::cout << "DISPATCH: \t\tSAFE TO COPY!\n";

      {
        std::scoped_lock lock(m_mutex);
        m_isSafe = true;
      }
    }

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
