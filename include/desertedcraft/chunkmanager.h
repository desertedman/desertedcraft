#pragma once

#include "chunk.h"
#include "gamestate.h"
#include <vector>

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

class ChunksList {
public:
  // ChunksList() : mChunkList(CHUNKLIST_SIZE) {}
  ChunksList() {
    mChunkList.reserve(CHUNK_DISTANCE * CHUNK_DISTANCE * CHUNK_DISTANCE);
  }

protected:
  std::vector<Chunk> mChunkList;
};

class ChunksLoadedList : public ChunksList {
public:
  /*
   * Store chunks in an array, with coordinates accessible by its 1D index?
   * Don't like this because it doesn't allow for negative coordinates
   *
   * Each chunk will have to store its own coords, offset by multiples of
   * CHUNK_SIZE_... This way each chunk can be dynamically loaded into
   * ChunkList, without care for its index
   *
   * What does adding/removing chunks look like?
   * Thinking in 2D top down:
   * Will need to calculate distance from center of each chunk; remove furthest
   * chunk from player, dynamically add new chunk to chunklist based on
   * whichever neighbor chunk is missing
   */

  /*
   * Imagine looking down at player from top down perspective, (X, Y)
   * 1. Iterate through surrounding directions of the center chunk and check
   * if each neighbor chunk is loaded. If not, generate new chunk
   * 2. Updating the list: When crossing a chunk, remove the furthest chunk.
   * Once again, iterate thru all directions and find whichever neighbor chunk
   * is missing
   */

  // TODO: Change this to take in a ref instead of a ptr
  ChunksLoadedList(const GameState *const gamestate);

  // Get origin of player's current chunk in chunk coordinates
  [[nodiscard]] const Voxel::vec3 GetPlayerChunkCoords();

  [[nodiscard]] int AddChunk(const int xChunkCoordOffset,
                             const int yChunkCoordOffset,
                             const int zChunkCoordOffset);

  [[nodiscard]] int InitChunks();

  const std::vector<Chunk> &GetChunksList() const;

private:
  const GameState *const mGameStatePtr;
};

class ChunksRenderedList : public ChunksList {};

class ChunkManager {
public:
private:
  std::vector<Chunk> ChunkList;
};
