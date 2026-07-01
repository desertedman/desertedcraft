#pragma once

#include "block.h"
#include "chunk.h"
#include "drawable.h"
#include "glad/glad.h"
#include "mesher.h"
#include <GLFW/glfw3.h>
#include <atomic>
#include <cassert>
#include <memory>
#include <mutex>
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
constexpr int CHUNK_DISTANCE = constexprPow(2, 2);
constexpr int RENDER_DISTANCE = constexprPow(2, 2);
static_assert(RENDER_DISTANCE <= CHUNK_DISTANCE,
              "Render distance must be less than or equal to chunk distance");

// Distance extended in all 3 axis
constexpr int FINAL_CHUNK_DISTANCE = constexprPow(CHUNK_DISTANCE, 3);
constexpr int FINAL_RENDER_DISTANCE = constexprPow(RENDER_DISTANCE, 3);

class ChunksDirtyList {
public:
  ChunksDirtyList() { mDirtyChunkPtrList.reserve(FINAL_CHUNK_DISTANCE); }

  void AddDirtyChunk(std::shared_ptr<Chunk> dirtyChunkPtr) {
    mDirtyChunkPtrList.push_back(dirtyChunkPtr);
  }

  void RebuildChunks(const GameState &gamestate) {
    mMeshesList.clear();
    mChunkinWorldCoordsList.clear();

    for (int i = 0; i < mDirtyChunkPtrList.size(); i++) {
      const auto &blocksPtr = mDirtyChunkPtrList[i]->GetBlocksPtr();
      mMeshesList.push_back(mesher.CreateMesh(blocksPtr));
      mChunkinWorldCoordsList.push_back(
          mDirtyChunkPtrList[i]->GetWorldCoords());
    }

    mDirtyChunkPtrList.clear();
  }

  const std::vector<std::shared_ptr<Chunk>> &GetChunksDirtyList() {
    return mDirtyChunkPtrList;
  }

private:
  std::vector<std::shared_ptr<Chunk>> mDirtyChunkPtrList;
  std::vector<glm::vec3> mChunkinWorldCoordsList;
  std::vector<std::shared_ptr<DrawableMesh>> mMeshesList;
  MesherNaive mesher;
};

class ChunksLoadedList {
public:
  ChunksLoadedList();

  // Get origin of player's current chunk in chunk coordinates
  [[nodiscard]] const glm::vec3
  GetPlayerChunkCoords(const GameState &gamestate) const;
  void AddChunk(const int xChunkCoordOffset, const int yChunkCoordOffset,
                const int zChunkCoordOffset);
  void AddChunk(const glm::vec3 &chunkCoordOffset);
  void Update(const GameState &gamestate);

  [[nodiscard]] std::shared_ptr<Chunk> MarkChunkDirty(const int xWorldCoord,
                                                      const int yWorldCoord,
                                                      const int zWorldCoord) {
    const glm::vec3 worldCoords(xWorldCoord, yWorldCoord, zWorldCoord);
    std::shared_ptr<Chunk> retPtr;
    int i = 0;

    for (; i < mChunkPtrList.size(); i++) {
      if (worldCoords == mChunkPtrList[i].get()->GetWorldCoords()) {
        retPtr = mChunkPtrList[i];
      }
    }

    return retPtr;
  }

  void TestChunkDelete(ChunksDirtyList &chunksDirtyList) {
    auto &chunk = *mChunkPtrList[0].get();

    for (int x = 0; x < CHUNK_SIZE_X; x++) {
      for (int y = 0; y < CHUNK_SIZE_Y; y++) {
        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
          chunk.SetBlock(BlockType_Air, x, y, z);
        }
      }
    }

    const auto worldCoords = chunk.GetWorldCoords();
    auto dirtyChunkPtr =
        MarkChunkDirty(worldCoords.x, worldCoords.y, worldCoords.z);
    chunksDirtyList.AddDirtyChunk(dirtyChunkPtr);
  }

  const std::vector<std::shared_ptr<Chunk>> &GetChunksList() const;

private:
  std::vector<std::shared_ptr<Chunk>> mChunkPtrList;
};

class ChunksRenderList {
public:
  ChunksRenderList();

  const std::vector<std::shared_ptr<DrawableMesh>> &GetMeshes() const;
  const std::vector<glm::vec3> &GetChunkWorldCoordsList() const;
  void Update(const ChunksLoadedList &chunks, const GameState &gamestate);

private:
  std::vector<std::shared_ptr<DrawableMesh>> mMeshesList;
  std::vector<glm::vec3> mChunkinWorldCoordsList;
  MesherNaive mMesher;
};

class ChunkManager {
public:
  [[nodiscard]] static glm::vec3
  ChunkToWorldCoords(const glm::vec3 &chunkCoords);
  [[nodiscard]] static glm::vec3
  WorldToChunkCoords(const glm::vec3 &worldCoords);

  ChunkManager(const GameState &gamestate);

  void Update();
  void TestChunkDelete() {
    mChunksLoadedList.TestChunkDelete(mChunksDirtyList);
  }
  const ChunksLoadedList &GetChunksLoadedList() const;
  const ChunksRenderList &GetChunksRenderList() const;
  void UpdateChunksLoadedList();
  void UpdateChunksRenderList();
  void Dispatch(std::mutex &renderMutex, std::atomic_bool &chunksListDirty,
                std::atomic_bool &dispatchRunning);

private:
  ChunksLoadedList mChunksLoadedList;
  ChunksRenderList mChunksRenderList;
  ChunksDirtyList mChunksDirtyList;
  const GameState &mGameState;
  glm::vec3 mOldPlayerChunkCoords;
};
