#pragma once

#include "block.h"
#include "drawable_mesh.h"
#include "mesher.h"
#include <glm/glm.hpp>
#include <vector>

// Assembles a mesh by looping through all a block's neighbors
class MesherNaive : public Mesher {
public:
  DrawableMesh CreateMesh(const Block ***const blocks) override;

private:
  void buildFace(const FaceDirection direction,
                 std::vector<glm::vec3> &vertices, const glm::vec3 offset);
};
