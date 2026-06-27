#include "mesher_naive.h"
#include "chunk.h"
#include "drawable.h"
#include "mesher.h"

DrawableMesh MesherNaive::CreateMesh(const Block ***const blocks) {
  std::vector<glm::vec3> vertices;

  for (int x = 0; x < CHUNK_SIZE_X; x++)
    for (int y = 0; y < CHUNK_SIZE_Y; y++)
      for (int z = 0; z < CHUNK_SIZE_Z; z++) {
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
          for (int axis = AXIS_X; axis < NUM_AXIS; axis++) {
            // Only one dimension will be incremented at a time
            const int index = currBlockCoords[axis] + dirVector[axis];

            int size = 0;
            if (axis == AXIS_X)
              size = CHUNK_SIZE_X;
            else if (axis == AXIS_Y)
              size = CHUNK_SIZE_Y;
            else if (axis == AXIS_Z)
              size = CHUNK_SIZE_Z;

            // If index is out of bounds, force building a face
            if (index < 0 || index == size) {
              checkNeighbor = false;
              buildFace(static_cast<FaceDirection>(i), vertices,
                        currBlockCoords);

              // Since we found the out of bounds index, we can skip the rest of
              // the check
              break;
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

  // NOTE: Make sure to invoke MOVE, not copy!!
  return DrawableMesh(vertices);
}
