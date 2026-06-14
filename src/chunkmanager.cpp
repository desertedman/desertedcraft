#include "chunkmanager.h"
#include <iostream>

// TODO: Change this to take in a ref instead of a ptr
ChunksLoadedList::ChunksLoadedList(const GameState *const gamestate)
    : mGameStatePtr(gamestate) {}

// Get origin of player's current chunk in chunk coordinates
[[nodiscard]] const Voxel::vec3 ChunksLoadedList::GetPlayerChunkCoords() {
  const auto &position = mGameStatePtr->GetConstCamera().Position;

  // Take camera's position and take integer division result
  const Voxel::vec3 coords(position.x / CHUNK_SIZE_X, position.y / CHUNK_SIZE_Y,
                           position.z / CHUNK_SIZE_Z);

  return coords;
}

// Returns 0 on successful allocation; -1 if allocation fails.
int ChunksLoadedList::AddChunk(const int xChunkCoordOffset,
                               const int yChunkCoordOffset,
                               const int zChunkCoordOffset) {
  /*
   * How do I add chunks AROUND the player?
   * Problem: Need to add chunks at some specific coordinates in the world.
   *
   * Lets look at it from 1D only.
   * In order to insert a chunk around the player, I need to multiply
   * CHUNK_SIZE_X by some offset (such as 0, 1, 2, -1...). Essentially need to
   * translate from CHUNK SPACE to WORLD SPACE coords
   */

  int xWorldCoord = xChunkCoordOffset * CHUNK_SIZE_X;
  int yWorldCoord = yChunkCoordOffset * CHUNK_SIZE_Y;
  int zWorldCoord = zChunkCoordOffset * CHUNK_SIZE_Z;

  // Must allocate new chunk on the heap, otherwise it will be deallocated
  // immediately after allocation
  Chunk *chunkPtr = new Chunk(xWorldCoord, yWorldCoord, zWorldCoord);
  if (!chunkPtr)
    return -1;

  mChunkList.push_back(*chunkPtr);

  return 0;
}

// Returns 0 on successful allocation; -1 if allocation fails.
int ChunksLoadedList::InitChunks() {
  auto playerChunkCoords = GetPlayerChunkCoords();

  for (int x = 0; x < CHUNK_DISTANCE; x++)
    for (int y = 0; y < CHUNK_DISTANCE; y++)
      for (int z = 0; z < CHUNK_DISTANCE; z++) {
        // Need to offset so that player spawns in the center of these chunks
        const int offset = CHUNK_DISTANCE / 2;

        const glm::vec3 dir(x - offset + playerChunkCoords.x,
                            y - offset + playerChunkCoords.y,
                            z - offset + playerChunkCoords.z);
        int ret = AddChunk(dir.x, dir.y, dir.z);

        if (ret != 0) {
          std::cout << "Failed to initiate chunk!\n";
          return -1;
        }
      }

  return 0;
}

const std::vector<Chunk> &ChunksLoadedList::GetChunksList() const {
  return mChunkList;
}
