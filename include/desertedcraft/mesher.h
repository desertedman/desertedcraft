#pragma once

#include "block.h"
#include "drawable_mesh.h"

class Mesher {
public:
  virtual DrawableMesh CreateMesh(Block ***blocks) = 0;
};
