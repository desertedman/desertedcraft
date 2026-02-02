#pragma once

#include "block.h"
#include <array>

constexpr int arraySize = 16;

class GameState {
public:
  std::array<std::array<std::array<Block, arraySize>, arraySize>, arraySize> m_pBlocks;

private:
};
