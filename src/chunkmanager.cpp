#include "chunkmanager.h"

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

void ChunksLoadedList::AddChunk(const int xChunkCoordOffset,
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

  // NOTE: Must allocate new chunk on the heap, otherwise it will be
  // deallocated immediately after allocation
  // TODO: Check for successful heap allocation; add error return code
  Chunk *chunkPtr = new Chunk(xWorldCoord, yWorldCoord, zWorldCoord);
  mChunkList.push_back(*chunkPtr);
}

void ChunksLoadedList::InitChunks() {
  // In order to init chunks around the player, we need access to the player's
  // position. We can get that through the GameState. Maybe we should separate
  // out the concept of a player and camera?

  auto playerChunkCoords = GetPlayerChunkCoords();
  playerChunkCoords.y = -1;

  // Init player's center chunk
  AddChunk(playerChunkCoords.x, playerChunkCoords.y, playerChunkCoords.z);

  // Init surrounding chunks
  for (int direction = Chunk_Right; direction < NUM_CHUNK_DIRS; direction++) {
    AddChunk(playerChunkCoords.x + chunkDirVectors[direction].x,
             playerChunkCoords.y + chunkDirVectors[direction].y,
             playerChunkCoords.z + chunkDirVectors[direction].z);
  }

  // TODO: Figure out loading chunks in diagonals around player
}

const std::vector<Chunk> &ChunksLoadedList::GetChunksList() const {
  return mChunkList;
}
