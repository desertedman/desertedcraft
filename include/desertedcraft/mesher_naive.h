#pragma once

#include "block.h"
#include "chunk.h"
#include "drawable_mesh.h"
#include "mesher.h"
#include <glm/ext/vector_float3.hpp>
#include <vector>

class MesherNaive : public Mesher {
public:
  DrawableMesh CreateMesh(Block ***blocks) override {
    std::vector<glm::vec3> vertices;

    for (int x = 0; x < CHUNK_SIZE; x++)
      for (int y = 0; y < CHUNK_SIZE; y++)
        for (int z = 0; z < CHUNK_SIZE; z++) {
          const auto &block = blocks[x][y][z];

          if (block.GetBlockType() == BlockType_Air)
            continue;

          // Have to iterate over an enum.... no clean way to do it
          // Iterate over all faces of cube
          for (int i = Right; i < NUM_FACES; i++) {
            const auto &dirVector = dirVectors[i];

            // Collect coords into a vec3 so we can iterate over them
            glm::vec3 currBlockCoords{x, y, z};
            bool checkNeighbor = true;

            // Edge case: if face is on a boundary, emit a face. MUST check
            // before accessing any blocks in the array to avoid out of bounds
            for (int j = 0; j < 3; j++) {
              const int currCoord =
                  currBlockCoords[j] + dirVector[j]; // Scalar coordinate
              if (currCoord < 0 || currCoord == CHUNK_SIZE) {
                checkNeighbor = false;
                buildFace(static_cast<FaceDirection>(i), vertices,
                          currBlockCoords);
              }
            }

            if (checkNeighbor) {
              int dirX = static_cast<int>(dirVector.x);
              int dirY = static_cast<int>(dirVector.y);
              int dirZ = static_cast<int>(dirVector.z);
              const auto &neighborBlock = blocks[x + dirX][y + dirY][z + dirZ];

              if (neighborBlock.GetBlockType() == BlockType_Air) {
                // may god smite me down for this code
                buildFace(static_cast<FaceDirection>(i), vertices,
                          currBlockCoords);
              }
            }
          }
        }

    DrawableMesh mesh(vertices);
    return mesh;
  }

private:
  void buildFace(const FaceDirection direction,
                 std::vector<glm::vec3> &vertices, const glm::vec3 offset) {
    // Take dirFace by ref so that it preserves array info
    const auto &dirFace = CubeFaces[direction];

    // Add all the vectors in dirFace to our vertices vector
    for (const auto &vec : dirFace) {
      const glm::vec3 newFace = vec + offset;

      vertices.push_back(newFace);
    }
  }
};
