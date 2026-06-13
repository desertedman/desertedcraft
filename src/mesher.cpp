#include "mesher.h"

void Mesher::buildFace(const FaceDirection direction,
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
