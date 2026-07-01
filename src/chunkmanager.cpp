#include "chunkmanager.h"
#include "chunk.h"
#include "gamestate.h"
#include "mesher.h"
#include <chrono>
#include <cmath>
#include <thread>

ChunksLoadedList::ChunksLoadedList() {
  mChunkPtrList.reserve(FINAL_CHUNK_DISTANCE);
}

// Get origin of player's current chunk in chunk coordinates
[[nodiscard]] const glm::vec3
ChunksLoadedList::GetPlayerChunkCoords(const GameState &gamestate) const {
  const auto &position = gamestate.GetConstCamera().Position;

  const auto playerChunkCoords = ChunkManager::WorldToChunkCoords(position);
  return playerChunkCoords;
}

void ChunksLoadedList::AddChunk(const int xChunkCoord, const int yChunkCoord,
                                const int zChunkCoord) {
  /*
   * How do I add chunks AROUND the player?
   * Problem: Need to add chunks at some specific coordinates in the world.
   *
   * Lets look at it from 1D only.
   * In order to insert a chunk around the player, I need to multiply
   * CHUNK_SIZE_X by some offset (such as 0, 1, 2, -1...). Essentially need to
   * translate from CHUNK SPACE to WORLD SPACE coords
   */

  const int xWorldCoord = xChunkCoord * CHUNK_SIZE_X;
  const int yWorldCoord = yChunkCoord * CHUNK_SIZE_Y;
  const int zWorldCoord = zChunkCoord * CHUNK_SIZE_Z;

  // Must allocate new chunk on the heap, otherwise it will be deallocated
  // immediately after allocation
  std::shared_ptr<Chunk> chunkPtr =
      std::make_shared<Chunk>(xWorldCoord, yWorldCoord, zWorldCoord);

  mChunkPtrList.push_back(chunkPtr);
}

void ChunksLoadedList::AddChunk(const glm::vec3 &chunkCoord) {
  AddChunk(static_cast<int>(chunkCoord.x), static_cast<int>(chunkCoord.y),
           static_cast<int>(chunkCoord.z));
}

// TODO: Make this run asynchronously!
void ChunksLoadedList::Update(const GameState &gamestate) {
  mChunkPtrList.clear();
  const auto &playerChunkCoords = GetPlayerChunkCoords(gamestate);

  for (int x = 0; x < CHUNK_DISTANCE; x++)
    for (int y = 0; y < CHUNK_DISTANCE; y++)
      for (int z = 0; z < CHUNK_DISTANCE; z++) {
        // Need to offset so that player spawns in the center of these chunks
        const glm::vec3 coords(x, y, z);

        glm::vec3 finalChunkCoords = coords + playerChunkCoords;
        // Center the CHUNK_DISTANCE around player
        const int centerOffset = CHUNK_DISTANCE / 2;
        finalChunkCoords -= static_cast<float>(centerOffset);
        AddChunk(finalChunkCoords);
      }
}

const std::vector<std::shared_ptr<Chunk>> &
ChunksLoadedList::GetChunksList() const {
  return mChunkPtrList;
}

ChunksRenderList::ChunksRenderList() {
  mMeshesList.reserve(FINAL_CHUNK_DISTANCE);
  mChunkinWorldCoordsList.reserve(FINAL_RENDER_DISTANCE);
}

const std::vector<std::shared_ptr<DrawableMesh>> &
ChunksRenderList::GetMeshes() const {
  return mMeshesList;
}

const std::vector<glm::vec3> &
ChunksRenderList::GetChunkWorldCoordsList() const {
  return mChunkinWorldCoordsList;
}

void ChunksRenderList::Update(const ChunksLoadedList &chunks,
                              const GameState &gamestate) {
  mMeshesList.clear();
  mChunkinWorldCoordsList.clear();
  const auto &playerChunkCoords = chunks.GetPlayerChunkCoords(gamestate);

  // std::cout << "Attempting to mesh\n";
  for (int x = 0; x < RENDER_DISTANCE; x++)
    for (int y = 0; y < RENDER_DISTANCE; y++)
      for (int z = 0; z < RENDER_DISTANCE; z++) {
        // Need to offset so that player spawns in the center of these chunks
        const glm::vec3 coords(x, y, z);

        glm::vec3 finalChunkCoords = coords + playerChunkCoords;
        // Center the CHUNK_DISTANCE around player
        const int centerOffset = RENDER_DISTANCE / 2;
        finalChunkCoords -= static_cast<float>(centerOffset);

        // Iterate over chunks list to find corresponding chunk
        const auto size = chunks.GetChunksList().size();
        for (int i = 0; i < size; i++) {
          const auto &currChunk = chunks.GetChunksList()[i].get();
          const auto currChunkCoords =
              ChunkManager::WorldToChunkCoords(currChunk->GetWorldCoords());

          if (currChunkCoords == finalChunkCoords) {
            mMeshesList.push_back(mMesher.CreateMesh(
                chunks.GetChunksList()[i].get()->GetBlocksPtr()));

            // NOTE: THIS NEEDS TO BE IN WORLD COORDS!!!
            mChunkinWorldCoordsList.push_back(currChunk->GetWorldCoords());
          }
        }
      }
  // std::cout << "All meshes assembled\n";
}

[[nodiscard]] glm::vec3
ChunkManager::ChunkToWorldCoords(const glm::vec3 &chunkCoords) {
  glm::vec3 retCoords(chunkCoords.x * CHUNK_SIZE_X,
                      chunkCoords.y * CHUNK_SIZE_Y,
                      chunkCoords.z * CHUNK_SIZE_Z);

  return retCoords;
}

[[nodiscard]] glm::vec3
ChunkManager::WorldToChunkCoords(const glm::vec3 &worldCoords) {
  glm::vec3 retCoords(worldCoords.x / CHUNK_SIZE_X,
                      worldCoords.y / CHUNK_SIZE_Y,
                      worldCoords.z / CHUNK_SIZE_Z);

  // TODO: Take a second look at this
  // Prefer taking -X, -Y, -Z of player position in closest chunk coords
  for (int i = 0; i < 3; i++) {
    // Because we want integer results, if we cast a negative decimal to int
    // (such as -0.5), it will become 0. The desired result is -1!
    // Instead, we can take the floor, which will give us desired results in
    // any case
    retCoords[i] = std::floor(retCoords[i]);
  }

  return retCoords;
}

ChunkManager::ChunkManager(const GameState &gamestate)
    : mGameState(gamestate),
      mOldPlayerChunkCoords(
          mChunksLoadedList.GetPlayerChunkCoords(mGameState)) {}

void ChunkManager::Update() {
  mChunksLoadedList.Update(mGameState);
  mChunksRenderList.Update(mChunksLoadedList, mGameState);
}

const ChunksLoadedList &ChunkManager::GetChunksLoadedList() const {
  return mChunksLoadedList;
}

const ChunksRenderList &ChunkManager::GetChunksRenderList() const {
  return mChunksRenderList;
}

void ChunkManager::UpdateChunksLoadedList() {
  mChunksLoadedList.Update(mGameState);
}

void ChunkManager::UpdateChunksRenderList() {
  mChunksRenderList.Update(mChunksLoadedList, mGameState);
}

void ChunkManager::Dispatch(std::mutex &renderMutex,
                            std::atomic_bool &chunksListDirty,
                            std::atomic_bool &dispatchRunning) {
  double hz = 20.f;
  auto interval = std::chrono::duration<double>(1.f / hz);
  using clock = std::chrono::steady_clock;
  auto next = clock::now();

  while (dispatchRunning) {
    const auto currPlayerChunkCoords =
        mChunksLoadedList.GetPlayerChunkCoords(mGameState);

    if (currPlayerChunkCoords != mOldPlayerChunkCoords) {
      mOldPlayerChunkCoords = currPlayerChunkCoords;
      chunksListDirty = true;

      std::lock_guard<std::mutex> guard(renderMutex);
      mChunksLoadedList.Update(mGameState);
    }

    next += std::chrono::duration_cast<clock::duration>(interval);
    std::this_thread::sleep_until(next);
  }
}
