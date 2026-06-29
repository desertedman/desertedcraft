#pragma once

#include "chunk.h"
#include "drawable.h"
#include "mesher.h"
#include <glm/ext/vector_float3.hpp>
#include <memory>
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
constexpr int CHUNK_DISTANCE = constexprPow(2, 3);
constexpr int RENDER_DISTANCE = constexprPow(2, 2);

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

  const std::vector<std::shared_ptr<Chunk>> &GetChunksList() const;

private:
  std::vector<std::shared_ptr<Chunk>> mChunkPtrList;
};

class ChunksRenderList {
public:
  ChunksRenderList();

  const std::vector<std::shared_ptr<DrawableMesh>> &GetMeshes() const { return mMeshesList; }
  const std::vector<glm::vec3> &GetChunkWorldCoordsList() const {
    return mChunkinWorldCoordsList;
  }
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
  const ChunksLoadedList &GetChunksLoadedList() const;
  const ChunksRenderList &GetChunksRenderList() const;
  void PollPlayerChunkCoords();

private:
  ChunksLoadedList mChunksLoadedList;
  ChunksRenderList mChunksRenderList;
  const GameState &mGameState;
  glm::vec3 mOldPlayerChunkCoords;
};
