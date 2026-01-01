#pragma once

#include "datagui/asset/uv_mesh.hpp"
#include "datagui/geometry/box.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/geometry/rot.hpp"
#include <vector>

namespace datagui {

class UvMeshShader {
public:
  void init();

  void queue_mesh(
      const UvMesh& uv_mesh,
      const Vec3& position,
      const Rot3& orientation,
      float opacity = 1);

  void draw(const Box2& viewport, const Camera3d& camera);
  void clear();

private:
  struct Command {
    UvMesh uv_mesh;
    Mat4 model_mat;
    float opacity;
  };
  std::vector<Command> commands;

  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_P;
  unsigned int uniform_V;
  unsigned int uniform_M;
  unsigned int uniform_opacity;
};

} // namespace datagui
