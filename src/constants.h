#pragma once

namespace Constants {
// These must be defined here because they are used in array definitions
inline constexpr int CHUNK_SIZE_X = 100; // horizontal
inline constexpr int CHUNK_SIZE_Z = 100; // depth
inline constexpr int CHUNK_SIZE_Y = 100; // vertical

inline constexpr int CHUNK_SIZE = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z;

extern const int CHUNK_DISTANCE_HORIZONTAL;
extern const int CHUNK_DISTANCE_VERTICAL;
// constexpr int RENDER_DISTANCE = constexprPow(2, 4);
// static_assert(RENDER_DISTANCE <= CHUNK_DISTANCE_HORIZONTAL,
//               "Render distance must be less than or equal to chunk
//               distance");

extern const int FINAL_CHUNK_DISTANCE;
extern const int FINAL_RENDER_DISTANCE;

// constexpr power function
// implicitly inline - wont violate ODR
constexpr int constexprPow(int base, int power) {
  int value = 1;

  for (int i = 0; i < power; i++)
    value *= base;

  return value;
}

extern const int NUM_THREADS;
extern const int NUM_WORKERS;
} // namespace Constants
