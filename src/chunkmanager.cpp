#include "chunkmanager.h"
#include "block.h"
#include "chunk.h"
#include "gamestate.h"
#include "mesher.h"
#include <algorithm>
#include <glm/ext/vector_int3.hpp>
#include <iostream>
#include <memory>

[[nodiscard]] glm::ivec3
ChunkManager::ChunkToWorldCoords(const glm::ivec3 &chunkCoords) {
  glm::ivec3 retCoords(chunkCoords.x * CHUNK_SIZE_X,
                       chunkCoords.y * CHUNK_SIZE_Y,
                       chunkCoords.z * CHUNK_SIZE_Z);

  return retCoords;
}

[[nodiscard]] glm::ivec3
ChunkManager::WorldToChunkCoords(const glm::ivec3 &worldCoords) {
  glm::ivec3 retCoords(worldCoords.x / CHUNK_SIZE_X,
                       worldCoords.y / CHUNK_SIZE_Y,
                       worldCoords.z / CHUNK_SIZE_Z);

  return retCoords;
}

ChunkManager::ChunkManager(const GameState &gamestate)
    : m_gameState(gamestate),
      m_oldPlayerChunkCoords(gamestate.GetPlayerChunkCoords()) {
  m_mesher = std::make_unique<MesherNaive>();
  // NOTE: Just reserved some arbitrary number
  m_chunksUnloadList.reserve(1000);
}

// Updates Render list
void ChunkManager::Update() {
  const auto playerChunkCoords = m_gameState.GetPlayerChunkCoords();

  // If render list is empty (e.g. if player just spawned), need to populate the
  // render list
  if (playerChunkCoords != m_oldPlayerChunkCoords ||
      m_chunksRenderList.empty()) {

    m_oldPlayerChunkCoords = playerChunkCoords;
    m_chunksRenderList.clear();

    // std::cout << "CHUNK LIST: \n";

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
          m_chunksRenderList.emplace_back(finalChunkCoords);
        }
  }

  int margin = 1;
  int maxDistance = CHUNK_DISTANCE_HORIZONTAL / 2 + margin;

  // Unload furthest chunks
  // NOTE: This for loop is an ITERATOR; as soon as we unload a chunk it becomes
  // INVALID!!! Solution: Collect all the coordinates to unload first, then loop
  // "Unload" over them
  for (const auto &[chunkPos, chunkPtr] : m_chunkMap) {
    auto posDiff = chunkPos - playerChunkCoords;

    // Distance from player chunk in each axis
    int dx = std::abs(posDiff.x);
    int dy = std::abs(posDiff.y);
    int dz = std::abs(posDiff.z);

    if (std::max(dx, dz) >= maxDistance) {
      m_chunksUnloadList.emplace_back(chunkPos);
    }

    // if (std::max({dx, dy, dz}) >= maxDistance) {
    //   m_chunksUnloadList.emplace_back(chunkPos);
    // }
  }

  for (const auto chunkPos : m_chunksUnloadList) {
    Unload(chunkPos);
  }

  m_chunksUnloadList.clear();
}

const Chunk &ChunkManager::GetChunk(const glm::ivec3 chunkCoordsPos) {
  auto iterator = m_chunkMap.find(chunkCoordsPos);
  const Chunk *retPtr;

  // Cached item found
  if (iterator != m_chunkMap.end()) {
    retPtr = iterator->second.get();
  }

  // Cached item not found - generate new item
  else {
    retPtr = GenerateChunk(chunkCoordsPos);
  }

  return *retPtr;
}

void ChunkManager::Unload(const glm::ivec3 pos) {
  auto iterator = m_chunkMap.find(pos);
  if (iterator == m_chunkMap.end()) {
    std::cerr << "TRIED TO UNLOAD CHUNK; DOES NOT EXIST\n";
    return;
  }

  else {
    m_chunkMap.erase(iterator);
  }
}

const std::vector<glm::ivec3> &ChunkManager::GetChunksRenderList() const {
  return m_chunksRenderList;
}

[[nodiscard]] Chunk *
ChunkManager::GenerateChunk(const glm::ivec3 &chunkCoordsPos) {
  // Must allocate new chunk on the heap, otherwise it will be deallocated
  // immediately after allocation
  auto chunkPtr = std::make_unique<Chunk>(chunkCoordsPos.x, chunkCoordsPos.y,
                                          chunkCoordsPos.z);

  // DEBUG: Set blocks higher than y = 0 to air
  if (chunkCoordsPos.y >= 0) {
    for (int x = 0; x < CHUNK_SIZE_X; x++)
      for (int y = 0; y < CHUNK_SIZE_Y; y++)
        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
          chunkPtr->SetBlock(BlockType::BlockType_Air, x, y, z);
        }
  }

  Chunk *rawChunkPtr = chunkPtr.get();

  auto meshPtr = m_mesher->CreateMesh(rawChunkPtr->GetBlocksPtr());
  rawChunkPtr->SetMesh(meshPtr);

  m_chunkMap.emplace(chunkCoordsPos, std::move(chunkPtr));

  return rawChunkPtr;
}
