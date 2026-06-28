#pragma once

#include "chunk.h"
#include "mesher_naive.h"
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

// TODO: Make separate draw distance constant

// Number of chunks to load around player. Must be a power of two in order to
// get nice cubic dimensions around the player; otherwise, the cube will be
// uneven. (ex. if player is at (0,0), then we want 2^2 = 4 nice even chunks
// surrounding the player
// TODO: Separate out chunk distance for x/z and y axis?
constexpr int CHUNK_DISTANCE = constexprPow(2, 3);

// TODO: Change ChunksLoadedList to use a map under the hood. Change
// ChunksRenderList to use set under the hood. That way they can be easily
// decoupled from each other (especially for draw distance). This way,
// ChunksRenderList can iterate AROUND the player's position, and index into
// ChunksLoadedList to get the actual chunk data. Otherwise, ChunksRenderList
// will have to iterate over ChunksLoadedList and find all the closest chunks,
// which is its own challenge....

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

  ChunksLoadedList();

  // Get origin of player's current chunk in chunk coordinates
  [[nodiscard]] const glm::vec3
  GetPlayerChunkCoords(const GameState &gamestate);
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

  const std::vector<DrawableMesh> &GetMeshes() const { return meshes; }
  void Update(const ChunksLoadedList &chunks);

private:
  std::vector<DrawableMesh> meshes;
  MesherNaive mesher;
};

class ChunkManager {
public:
  ChunkManager(const GameState &gamestate) : mGameState((gamestate)) {}

  void Update();
  const ChunksLoadedList &GetChunksLoadedList() const;
  const ChunksRenderList &GetChunksRenderList() const;

private:
  ChunksLoadedList mChunksLoadedList;
  ChunksRenderList mChunksRenderList;
  const GameState &mGameState;
};
