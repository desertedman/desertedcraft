#pragma once

#include "block.h"
#include "mesh.h"
#include <glm/vec3.hpp>
#include <memory>

constexpr int CHUNK_SIZE_X = 100; // horizontal
constexpr int CHUNK_SIZE_Z = 100; // depth
constexpr int CHUNK_SIZE_Y = 100; // vertical

class Chunk {
public:
  Chunk(const int xCoord = 0, const int yCoord = 0, const int zCoord = 0);
  Chunk(const glm::ivec3 &coords);

  Chunk(const Chunk &other);
  Chunk &operator=(const Chunk &other);

  Chunk(Chunk &&other) noexcept;
  Chunk &operator=(Chunk &&other) noexcept;

  ~Chunk();

  void SetBlock(const BlockType blockType, const int xCoord, const int yCoord,
                const int zCoord);
  void SetMesh(std::unique_ptr<Mesh> &meshPtr);
  Mesh *GetMeshPtr() const { return m_meshPtr.get(); }
  const Block &GetBlock(const int x, const int y, const int z) const;
  const Block ***const GetBlocksPtr() const;
  [[nodiscard]] const glm::ivec3 GetWorldCoords() const;

private:
  void DeleteChunk();
  void CreateChunk(const int xCoord = 0, const int yCoord = 0,
                   const int zCoord = 0);

  Block ***m_blocksPtr;
  std::unique_ptr<Mesh> m_meshPtr;
  glm::ivec3 m_worldCoords;
};
