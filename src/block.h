#pragma once

enum BlockType {
  BlockType_Default = 0,
  BlockType_Air,
  BlockType_Grass,
  BlockType_Dirt,
  BlockType_Water,
  BlockType_Stone,
  BlockType_Wood,
  BlockType_Sand,
  BlockType_NumTypes,
};

class Block {
public:
  Block();
  const BlockType GetBlockType() const;
  void SetBlockType(const BlockType blockType);

  bool isActive;

private:
  BlockType blockType;
};
