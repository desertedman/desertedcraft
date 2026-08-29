#include "chunkmanager.h"
#include "chunk.h"
#include "gamestate.h"
#include "mesher.h"
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
    : mGameState(gamestate),
      mOldPlayerChunkCoords(gamestate.GetPlayerChunkCoords()) {}

// Updates Render list
void ChunkManager::Update() {
  const auto playerChunkCoords = mGameState.GetPlayerChunkCoords();

  // If render list is empty (e.g. if player just spawned), need to populate the
  // render list
  if (playerChunkCoords != mOldPlayerChunkCoords || mChunksRenderList.empty()) {
    // std::cout << "PLAYER CHUNK COORDS: " << playerChunkCoords.x << " "
    //           << playerChunkCoords.y << " " << playerChunkCoords.z << "\n";

    mOldPlayerChunkCoords = playerChunkCoords;
    mChunksRenderList.clear();

    // std::cout << "CHUNK LIST: \n";

    for (int x = 0; x < CHUNK_DISTANCE; x++)
      for (int y = 0; y < CHUNK_DISTANCE; y++)
        for (int z = 0; z < CHUNK_DISTANCE; z++) {
          // Need to offset so that player spawns in the center of these chunks
          const glm::ivec3 coords(x, y, z);

          glm::ivec3 finalChunkCoords = coords + playerChunkCoords;
          // Center the CHUNK_DISTANCE around player
          const int centerOffset = CHUNK_DISTANCE / 2;
          finalChunkCoords = finalChunkCoords - centerOffset;

          // Add chunk to the render list
          mChunksRenderList.emplace_back(finalChunkCoords);

          // std::cout << finalChunkCoords.x << " " << finalChunkCoords.y << " "
          //           << finalChunkCoords.z << "\n";
        }
  }
}

const std::vector<glm::ivec3> &ChunkManager::GetChunksRenderList() const {
  return mChunksRenderList;
}
Chunk *ChunkCache::Get(const glm::ivec3 chunkCoordsPos) {
  auto iterator = mChunkMap.find(chunkCoordsPos);
  Chunk *retPtr;

  // Cached item found
  if (iterator != mChunkMap.end()) {
    retPtr = iterator->second.get();
  }

  // Cached item not found - generate new item
  else {
    retPtr = GenerateChunk(chunkCoordsPos);
  }

  return retPtr;
}
void ChunkCache::Unload(const glm::ivec3 pos) {
  auto iterator = mChunkMap.find(pos);
  if (iterator == mChunkMap.end())
    return;

  else {
    mChunkMap.erase(pos);
  }
}
Chunk *ChunkCache::GenerateChunk(const glm::ivec3 &chunkCoordsPos) {
  // Must allocate new chunk on the heap, otherwise it will be deallocated
  // immediately after allocation
  auto chunkPtr = std::make_unique<Chunk>(chunkCoordsPos.x, chunkCoordsPos.y,
                                          chunkCoordsPos.z);
  Chunk *rawChunkPtr = chunkPtr.get();

  auto meshPtr = mMesher->CreateMesh(rawChunkPtr->GetBlocksPtr());
  rawChunkPtr->SetMesh(meshPtr);

  mChunkMap.emplace(chunkCoordsPos, std::move(chunkPtr));

  return rawChunkPtr;
}
