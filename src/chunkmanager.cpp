#include "chunkmanager.h"
#include "chunk.h"
#include "gamestate.h"
#include "mesher_naive.h"
#include <cmath>

// TODO: Change this to take in a ref instead of a ptr
ChunksLoadedList::ChunksLoadedList(const GameState *const gamestate)
    : mGameStatePtr(gamestate) {
  mChunkPtrList.reserve(CHUNK_DISTANCE * CHUNK_DISTANCE * CHUNK_DISTANCE);
}

// Get origin of player's current chunk in chunk coordinates
[[nodiscard]] const glm::vec3 ChunksLoadedList::GetPlayerChunkCoords() {
  const auto &position = mGameStatePtr->GetConstCamera().Position;

  // Translate camera's position to chunk coords
  glm::vec3 chunkCoords(position.x / CHUNK_SIZE_X, position.y / CHUNK_SIZE_Y,
                        position.z / CHUNK_SIZE_Z);

  // TODO: Take a second look at this
  // Prefer taking -X, -Y, -Z of player position in closest chunk coords
  for (int i = 0; i < 3; i++) {
    // Because we want integer results, if we cast a negative decimal to int
    // (such as -0.5), it will become 0. The desired result is -1!
    // Instead, we can take the floor, which will give us desired results in any
    // case
    chunkCoords[i] = std::floor(chunkCoords[i]);
  }

  return chunkCoords;
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

  const int xWorldCoord = xChunkCoordOffset * CHUNK_SIZE_X;
  const int yWorldCoord = yChunkCoordOffset * CHUNK_SIZE_Y;
  const int zWorldCoord = zChunkCoordOffset * CHUNK_SIZE_Z;

  // Must allocate new chunk on the heap, otherwise it will be deallocated
  // immediately after allocation
  std::shared_ptr<Chunk> chunkPtr =
      std::make_shared<Chunk>(xWorldCoord, yWorldCoord, zWorldCoord);

  mChunkPtrList.push_back(chunkPtr);
}

void ChunksLoadedList::AddChunk(const glm::vec3 &chunkCoordOffset) {
  AddChunk(chunkCoordOffset.x, chunkCoordOffset.y, chunkCoordOffset.z);
}

void ChunksLoadedList::InitChunks() {
  auto playerChunkCoords = GetPlayerChunkCoords();

  for (int x = 0; x < CHUNK_DISTANCE; x++)
    for (int y = 0; y < CHUNK_DISTANCE; y++)
      for (int z = 0; z < CHUNK_DISTANCE; z++) {
        // Need to offset so that player spawns in the center of these chunks
        const glm::vec3 coords(x, y, z);
        const int offset = CHUNK_DISTANCE / 2;

        glm::vec3 dir = coords + playerChunkCoords;
        dir -= offset;
        AddChunk(dir);
      }
}

const std::vector<std::shared_ptr<Chunk>> &
ChunksLoadedList::GetChunksList() const {
  return mChunkPtrList;
}

// TODO: This needs to run asynchronously!
void ChunksLoadedList::Update() {
  mChunkPtrList.clear();
  InitChunks();
}

void ChunksRenderList::Update(const ChunksLoadedList &chunks) {
  auto asdf = chunks.GetChunksList();

  // Iterate over list and mesh chunks
  MesherNaive mesher;
  int size = chunks.GetChunksList().size();
  for (int i = 0; i < size; i++) {
  }
}
