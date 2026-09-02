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
  // TODO: Create a "using" alias for this horrendous pointer
  [[nodiscard]] virtual std::vector<glm::vec3>
  CreateVertices(const Block *const *const *const blocks) = 0;

  virtual ~Mesher() = default;

protected:
  // NOTE: may need to be marked virtual in the future
  void BuildFace(const FaceDirection direction,
                 std::vector<glm::vec3> &vertices, const glm::vec3 offset);
};

// Assembles a complete mesh with no optimization
class MesherBasic : public Mesher {
public:
  std::unique_ptr<Mesh> CreateMesh(const Block ***const blocks) override;
  [[nodiscard]] std::vector<glm::vec3>
  CreateVertices(const Block *const *const *const blocks) override {
    assert("FUNCTION NOT IMPLEMENTED\n");

    return std::vector<glm::vec3>(2);
  }
};

// Assembles a mesh by looping through all a block's neighbors
class MesherNaive : public Mesher {
public:
  // TODO: Must change to return a std::vector of vertices
  [[nodiscard]] std::unique_ptr<Mesh>
  CreateMesh(const Block ***const blocks) override;
  [[nodiscard]] std::vector<glm::vec3>
  CreateVertices(const Block *const *const *const blocks) override;
};
