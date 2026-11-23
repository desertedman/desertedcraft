#pragma once

enum class BlockType {
  Default = 0,
  Air,
  Grass,
  Dirt,
  Water,
  Stone,
  Wood,
  Sand,
  NUM_TYPES,
};

class Block {
public:
  Block();
  Block(const Block &other) = default;
  Block(Block &&other) = default;
  Block &operator=(const Block &other) = default;
  Block &operator=(Block &&other) = default;

  BlockType GetBlockType() const;
  void SetBlockType(const BlockType blockType);

private:
  BlockType m_blockType;
};
