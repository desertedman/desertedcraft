#pragma once

#include "chunk.h"
#include <memory>
#include <vector>

// Forward declare to resolve circular dependency
class GameState;

// constexpr power function
constexpr int pow(int base, int power) {
  int value = 1;

  for (int i = 0; i < power; i++)
    value *= power;

  return value;
}

// TODO: Separate out chunk distance for x/z and y axis?
// Must be a power of two!
constexpr int CHUNK_DISTANCE = pow(2, 2);

class ChunksLoadedList {
public:
  /*
   * Store chunks in an array, with coordinates accessible by its 1D index?
   * Don't like this because it doesn't allow for negative coordinates
   *
   * Each chunk will have to store its own coords, offset by multiples of
   * CHUNK_SIZE_... This way each chunk can be dynamically loaded into
   * ChunkList, without care for its index
   */

  // TODO: Change this to take in a ref instead of a ptr
  ChunksLoadedList(const GameState *const gamestate);

  // Get origin of player's current chunk in chunk coordinates
  [[nodiscard]] const glm::vec3 GetPlayerChunkCoords();
  void AddChunk(const int xChunkCoordOffset, const int yChunkCoordOffset,
                const int zChunkCoordOffset);
  void AddChunk(const glm::vec3 &chunkCoordOffset);
  void InitChunks();
  void Update();

  const std::vector<std::shared_ptr<Chunk>> &GetChunksList() const;

private:
  // TODO: Change this to take in a ref instead of a ptr
  const GameState *const mGameStatePtr;
  std::vector<std::shared_ptr<Chunk>> mChunkPtrList;
};

class ChunksRenderList {
public:
  ChunksRenderList(const ChunksLoadedList &chunks) {
    const int size = chunks.GetChunksList().size();
    meshes.reserve(size);
  }

  void Update(const ChunksLoadedList &chunks);

private:
  std::vector<DrawableMesh> meshes;
};

class ChunkManager {
public:
private:
  std::vector<Chunk> ChunkList;
};
