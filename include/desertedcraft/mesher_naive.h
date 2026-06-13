#pragma once

#include "block.h"
#include "drawable_mesh.h"
#include "mesher.h"

// Assembles a mesh by looping through all a block's neighbors
class MesherNaive : public Mesher {
public:
  DrawableMesh CreateMesh(const Block ***const blocks) override;
};
