#pragma once

#include "block.h"
#include "drawable.h"
#include "mesher.h"

// Assembles a complete mesh with no optimization
class MesherBasic : public Mesher {
public:
  std::shared_ptr<DrawableMesh> CreateMesh(const Block ***const blocks) override;
};
