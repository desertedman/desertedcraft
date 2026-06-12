#pragma once

#include "block.h"
#include "drawable_mesh.h"
#include "mesher.h"

// Assembles a complete mesh with no optimization
class MesherBasic : Mesher {
public:
  DrawableMesh CreateMesh(Block ***blocks) override;

private:
  void buildFace(const FaceDirection direction,
                 std::vector<glm::vec3> &vertices, const glm::vec3 offset);
};
