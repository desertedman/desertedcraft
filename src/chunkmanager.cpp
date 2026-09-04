#include "chunkmanager.h"
#include "block.h"
#include "chunk.h"
#include "gamestate.h"
#include "mesher.h"
#include <atomic>
#include <cmath>
#include <iostream>
#include <memory>
#include <mutex>
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
      m_oldPlayerChunkCoords(gamestate.GetPlayerChunkCoords()),
      m_noiseData(std::vector<int>(CHUNK_SIZE_X * CHUNK_SIZE_Z)),
      m_isDirty(true) {
  m_mesherPtr = std::make_unique<MesherNaive>();
  // NOTE: Just reserved some arbitrary number
  m_chunksUnloadList.reserve(1000);

  m_noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);

  // TODO: NOISE VALUES MUST BE NORMALIZED
  int index = 0;
  for (int y = 0; y < CHUNK_SIZE_Z; y++)
    for (int x = 0; x < CHUNK_SIZE_X; x++) {
      auto noise = m_noise.GetNoise((float)x, (float)y);

      // Transform noise from (-1, 1) to (0, 1)
      noise += 1;
      noise /= 2;

      // std::cout << "NOISE: " << noise << "\n";
      m_noiseData[index++] = noise * CHUNK_SIZE_Y;
    }

  // for (const auto blah : m_noiseData)
  //   std::cout << "NOISE: " << blah << "\n";
}

// Updates Render list
void ChunkManager::Update() {
  if (m_isDirty) {
    if (m_isSafe) {
      // NOTE: This possibly straight up does nothing because we're not locking
      // anywhere else
      std::scoped_lock(mutex);

      // Copy lists to main thread
      m_chunksRenderList = m_dispatchChunksRenderList;

      for (auto vec : m_chunksRenderList) {
        const auto iterator = m_chunkMap.find(vec);

        // Move chunk from dispatch map to main map
        if (iterator == m_chunkMap.end()) {
          auto &dispatchChunk = m_dispatchChunkMap.find(vec)->second;
          m_chunkMap.emplace(vec, std::move(dispatchChunk));
        }
      }

      m_isDirty = false;
      m_dispatchChunkMap.clear();
      std::cout << "MAIN: MOVED TO MAIN THREAD\n";
      std::cout << "MAIN: DISPATCH LIST SIZE = "
                << m_dispatchChunksRenderList.size() << "\n";
      std::cout << "MAIN: DISPATCH MAP SIZE = " << m_dispatchChunkMap.size()
                << "\n";
      std::cout << "MAIN: MAIN MAP SIZE = " << m_chunkMap.size() << "\n";
    }
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
  // NOTE: This for loop is an ITERATOR; as soon as we unload a chunk it becomes
  // INVALID!!! Solution: Collect all the coordinates to unload first, then loop
  // "Unload" over them
  for (const auto &[chunkPos, chunkPtr] : m_chunkMap) {
    auto posDiff = chunkPos - m_oldPlayerChunkCoords;

    // Distance from player chunk in each axis
    int dx = std::abs(posDiff.x);
    int dy = std::abs(posDiff.y);
    int dz = std::abs(posDiff.z);

    if (std::max(dx, dz) >= maxDistance) {
      m_chunksUnloadList.emplace_back(chunkPos);
    }
  }

  for (const auto chunkPos : m_chunksUnloadList) {
    Unload(chunkPos);
    std::cout << "MAIN: CHUNK UNLOADED\n";
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
  int index = 0;
  for (int x = 0; x < CHUNK_SIZE_X; x++) {
    for (int z = 0; z < CHUNK_SIZE_Z; z++) {
      for (int y = 0; y < CHUNK_SIZE_Y; y++) {
        if (y > m_noiseData[index]) {
          chunkPtr->SetBlock(BlockType::BlockType_Air, x, y, z);
        }
      }

      index++;
    }
  }

  Chunk *rawChunkPtr = chunkPtr.get();

  auto meshPtr = m_mesherPtr->CreateMesh(rawChunkPtr->GetBlocksPtr());
  rawChunkPtr->SetMesh(meshPtr);

  return chunkPtr;
}

// TODO: Add unloading logic
void ChunkManager::Dispatch(std::atomic_bool &running) {
  while (running) {
    auto currPlayerChunkCoords = m_gameState.GetPlayerChunkCoords();

    // If render list is empty, need to populate it
    if (currPlayerChunkCoords != m_oldPlayerChunkCoords ||
        m_dispatchChunksRenderList.empty()) {
      std::cout << "DISPATCH: RENDER LIST DIRTY\n";

      std::scoped_lock(mutex);

      m_isDirty = true;
      m_isSafe = false;

      // Update player coords
      m_oldPlayerChunkCoords = currPlayerChunkCoords;

      // Build new render list
      m_dispatchChunksRenderList.clear();
      BuildRenderList(currPlayerChunkCoords, m_dispatchChunksRenderList);
      std::cout << "DISPATCH: RENDER LIST POPULATED\n";

      // Generate each chunk and corresponding mesh
      for (const auto vec : m_dispatchChunksRenderList) {
        auto iterator = m_chunkMap.find(vec);

        // Cached chunk not found - generate new chunk
        if (iterator == m_chunkMap.end()) {
          auto newChunkPtr = GenerateChunk(vec);

          m_dispatchChunkMap.emplace(vec, std::move(newChunkPtr));
          std::cout << "DISPATCH: GENERATED NEW CHUNK\n";
        }

        // Cached chunk found in main map
        else {
          std::cout << "DISPATCH: CACHED CHUNK FOUND IN MAIN MAP\n";
          continue;
        }
      }

      std::cout << "DISPATCH: LOADED ALL CHUNKS\n";
      std::cout << "DISPATCH: \t\tSAFE TO COPY!\n";

      m_isSafe = true;
    }
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
