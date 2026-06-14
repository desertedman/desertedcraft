#pragma once

#include "block.h"
#include "drawable.h"
#include "mesher.h"

// Assembles a complete mesh with no optimization
class MesherBasic : public Mesher {
public:
  DrawableMesh CreateMesh(const Block ***const blocks) override;
};
