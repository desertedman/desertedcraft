#pragma once

#include "block.h"
#include "cube.h"
#include "mesh.h"
#include <cassert>
#include <memory>

class Mesher {
public:
  [[nodiscard]] virtual std::unique_ptr<Mesh>
  CreateMesh(const Block ***const blocks) = 0;

  virtual ~Mesher() = default;

protected:
  // NOTE: may need to be marked virtual in the future
  void BuildFace(const FaceDirection direction,
                 std::vector<glm::vec3> &vertices, const glm::vec3 offset);
};

// Assembles a complete mesh with no optimization
class MesherBasic : public Mesher {
public:
  [[nodiscard]] std::unique_ptr<Mesh>
  CreateMesh(const Block ***const blocks) override;
};

// Assembles a mesh by looping through all a block's neighbors
class MesherNaive : public Mesher {
public:
  // TODO: Must change to return a std::vector of vertices
  [[nodiscard]] std::unique_ptr<Mesh>
  CreateMesh(const Block ***const blocks) override;
};
