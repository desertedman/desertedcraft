#include "mesher_basic.h"
#include "chunk.h"

std::shared_ptr<DrawableMesh>
MesherBasic::CreateMesh(const Block ***const blocks) {
  std::vector<glm::vec3> vertices;

  for (int x = 0; x < CHUNK_SIZE_X; x++)
    for (int y = 0; y < CHUNK_SIZE_Y; y++)
      for (int z = 0; z < CHUNK_SIZE_Z; z++) {
        const auto &block = blocks[x][y][z];

        if (block.GetBlockType() == BlockType_Air)
          continue;

        glm::vec3 currBlockCoords{x, y, z};

        for (int i = Right; i < NUM_FACES; i++) {
          buildFace(static_cast<FaceDirection>(i), vertices, currBlockCoords);
        }
      }

  std::shared_ptr<DrawableMesh> meshPtr =
      std::make_shared<DrawableMesh>(vertices);
  return meshPtr;
};
