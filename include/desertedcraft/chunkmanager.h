#pragma once

#include "chunk.h"
#include "gamestate.h"
#include <vector>

/*
 * Store chunks in an array, with coordinates accessible by its 1D index?
 * Don't like this because it doesn't allow for negative coordinates
 *
 * Each chunk will have to store its own coords, offset by multiples of
 * CHUNK_SIZE_... This way each chunk can be dynamically loaded into ChunkList,
 * without care for its index
 *
 * Maybe want to store maximum of 9 chunks at a time;
 * 1 chunk for the center of the player, and 8 that surround the rest of the
 * player
 *
 * What does adding/removing chunks look like?
 * Thinking in 2D top down:
 * Will need to calculate distance from center of each chunk; remove furthest
 * chunk from player, dynamically add new chunk to chunklist based on whichever
 * neighbor chunk is missing
 */

// TODO: replace instances of these enums with those in mesher.h
enum ChunkDirection {
  Chunk_Right,
  Chunk_Left,
  Chunk_Front,
  Chunk_Back,
  NUM_CHUNK_DIRS,
};

const Voxel::vec3 chunkDirVectors[4] = {
    {1, 0, 0},  // Right
    {-1, 0, 0}, // Left
    {0, 0, 1},  // Front
    {0, 0, -1}  // Back
};

constexpr int CHUNKLIST_SIZE = 9;

class ChunksList {
public:
  // ChunksList() : mChunkList(CHUNKLIST_SIZE) {}
  ChunksList() { mChunkList.reserve(CHUNKLIST_SIZE); }

protected:
  std::vector<Chunk> mChunkList;
};

class ChunksLoadedList : public ChunksList {
public:
  /*
   * Imagine looking down at player from top down perspective, (X, Y)
   * 1. Initiate chunks in a 3x3 grid around the player
   * 2. If player is on a boundary (say at 0,0), then choose (+X, +Y) chunk as
   * center chunk THIS IS ALREADY TAKEN CARE OF
   * 3. Iterate through surrounding directions of the center chunk and check
   * if each neighbor chunk is loaded. If not, generate new chunk
   * 4. Updating the list: When crossing a chunk, remove the furthest chunk.
   * Once again, iterate thru all directions and find whichever neighbor chunk
   * is missing
   */

  // TODO: Change this to take in a ref instead of a ptr
  ChunksLoadedList(const GameState *const gamestate);

  // Get origin of player's current chunk in chunk coordinates
  [[nodiscard]] const Voxel::vec3 GetPlayerChunkCoords();

  void AddChunk(const int xChunkCoordOffset, const int yChunkCoordOffset,
                const int zChunkCoordOffset);

  void InitChunks();

  const std::vector<Chunk> &GetChunksList() const;

private:
  const GameState *const mGameStatePtr;
};

class ChunkManager {
public:
private:
  std::vector<Chunk> ChunkList;
};
