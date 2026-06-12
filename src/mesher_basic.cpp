#include "mesher_basic.h"
#include "chunk.h"

DrawableMesh MesherBasic::CreateMesh(const Block ***const blocks) {
  std::vector<glm::vec3> vertices;

  for (int x = 0; x < CHUNK_SIZE; x++)
    for (int y = 0; y < CHUNK_SIZE; y++)
      for (int z = 0; z < CHUNK_SIZE; z++) {
        const auto &block = blocks[x][y][z];

        if (block.GetBlockType() == BlockType_Air)
          continue;

        glm::vec3 currBlockCoords{x, y, z};

        for (int i = Right; i < NUM_FACES; i++) {
          buildFace(static_cast<FaceDirection>(i), vertices, currBlockCoords);
        }
      }

  DrawableMesh mesh(vertices);
  return mesh;
};

void MesherBasic::buildFace(const FaceDirection direction,
                            std::vector<glm::vec3> &vertices,
                            const glm::vec3 offset) {
  // Take dirFace by ref so that it preserves array info
  const auto &dirFace = CubeFaces[direction];

  // Add all the vectors in dirFace to our vertices vector
  for (const auto &vec : dirFace) {
    const glm::vec3 newFace = vec + offset;

    vertices.push_back(newFace);
  }
}
