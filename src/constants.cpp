#include "constants.h"

namespace Constants {
// Number of chunks to load around player. Must be a power of two in order to
// get nice cubic dimensions around the player; otherwise, the cube will be
// uneven. (ex. if player is at (0,0), then we want 2^2 = 4 nice even chunks
// surrounding the player
extern constexpr int CHUNK_DISTANCE_HORIZONTAL = constexprPow(2, 3);
extern constexpr int CHUNK_DISTANCE_VERTICAL = 1;

// Distance extended in all 3 axis
// NOTE: USE ONLY FOR RESERVING SPACE!! NOT FOR LOOPS OR ITERATIONS
extern constexpr int FINAL_CHUNK_DISTANCE = CHUNK_DISTANCE_HORIZONTAL *
                                            CHUNK_DISTANCE_HORIZONTAL *
                                            CHUNK_DISTANCE_VERTICAL;
extern constexpr int FINAL_RENDER_DISTANCE = FINAL_CHUNK_DISTANCE;

// For debug builds:
// 9-10 threads seems to be fastest for Linux; 4 threads for Windows
extern constexpr int NUM_THREADS = 10;
extern constexpr int NUM_WORKERS = NUM_THREADS - 1;
static_assert(NUM_THREADS >= 2, "NUM_THREADS MUST BE >= 2");
} // namespace Constants
