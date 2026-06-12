#pragma once

#include "block.h"
#include "drawable_mesh.h"
#include "mesher.h"
#include <glm/ext/vector_float3.hpp>
#include <vector>

class MesherNaive : public Mesher {
public:
  DrawableMesh CreateMesh(Block ***blocks) override;

private:
  void buildFace(const FaceDirection direction,
                 std::vector<glm::vec3> &vertices, const glm::vec3 offset);
};
