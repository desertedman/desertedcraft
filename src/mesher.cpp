#include "mesher.h"
#include "block.h"
#include "chunk.h"

void Mesher::BuildFace(const FaceDirection direction,
                       std::vector<glm::vec3> &vertices,
                       const glm::vec3 offset) {
  // Take dirFace by ref so that it preserves array info
  const auto &dirFace = CubeFaces[direction];

  // Reserve additional space for incoming vertices
  vertices.reserve(vertices.size() + dirFace->length());

  // Add all the vectors in dirFace to our vertices vector
  for (const auto &vec : dirFace) {
    const glm::vec3 newFace = vec + offset;

    vertices.push_back(newFace);
  }
}

std::unique_ptr<Mesh> MesherBasic::CreateMesh(const Block ***const blocks) {
  std::vector<glm::vec3> vertices;

  for (int x = 0; x < CHUNK_SIZE_X; x++)
    for (int y = 0; y < CHUNK_SIZE_Y; y++)
      for (int z = 0; z < CHUNK_SIZE_Z; z++) {
        const auto &block = blocks[x][y][z];

        if (block.GetBlockType() == BlockType::BlockType_Air)
          continue;

        glm::vec3 currBlockCoords{x, y, z};

        for (int i = FACE_RIGHT; i < NUM_FACES; i++) {
          BuildFace(static_cast<FaceDirection>(i), vertices, currBlockCoords);
        }
      }

  auto meshPtr = std::make_unique<Mesh>(vertices);
  return meshPtr;
};

std::unique_ptr<Mesh> MesherNaive::CreateMesh(const Block ***const blocks) {
  std::vector<glm::vec3> vertices;

  for (int x = 0; x < CHUNK_SIZE_X; x++)
    for (int y = 0; y < CHUNK_SIZE_Y; y++)
      for (int z = 0; z < CHUNK_SIZE_Z; z++) {
        const auto &block = blocks[x][y][z];

        if (block.GetBlockType() == BlockType::BlockType_Air)
          continue;

        // Have to iterate over an enum.... no clean way to do it
        // Iterate over all faces of cube
        for (int currFace = FACE_RIGHT; currFace < NUM_FACES; currFace++) {
          const glm::ivec3 &currVector = dirVectors[currFace];

          // Collect coords into a vec3 so we can iterate over them
          glm::ivec3 currBlockCoords{x, y, z};
          bool checkNeighbor = true;

          // Edge case: if face is on a boundary, emit a face. MUST check
          // before accessing any blocks in the array to avoid out of bounds
          for (int currCoord = COORDINATE_X; currCoord < NUM_COORDINATES;
               currCoord++) {
            // Only one dimension will be incremented at a time
            const int index =
                currBlockCoords[currCoord] + currVector[currCoord];

            // Which chunk-axis (or "coordinate") boundary are we checking for?
            // Ex. if we're checking the X coordinate boundary, then we set
            // "size" to the span of the chunk X length. Later on, we use "size"
            // to determine if we're lying on the edge of a chunk.
            int size = 0;
            if (currCoord == COORDINATE_X)
              size = CHUNK_SIZE_X;
            else if (currCoord == COORDINATE_Y)
              size = CHUNK_SIZE_Y;
            else if (currCoord == COORDINATE_Z)
              size = CHUNK_SIZE_Z;

            // If index is out of bounds, force building a face
            if (index < 0 || index == size) {
              checkNeighbor = false;
              BuildFace(static_cast<FaceDirection>(currFace), vertices,
                        currBlockCoords);

              // Since we found the out of bounds index, we can skip the rest of
              // the check
              break;
            }
          }

          if (checkNeighbor) {
            int dirX = currVector.x;
            int dirY = currVector.y;
            int dirZ = currVector.z;
            const auto &neighborBlock = blocks[x + dirX][y + dirY][z + dirZ];

            if (neighborBlock.GetBlockType() == BlockType::BlockType_Air) {
              // may god smite me down for this code
              BuildFace(static_cast<FaceDirection>(currFace), vertices,
                        currBlockCoords);
            }
          }
        }
      }

  auto meshPtr = std::make_unique<Mesh>(vertices);
  return meshPtr;
}
