#pragma once

#include "datagui/asset/mesh.hpp"
#include "datagui/color.hpp"
#include "datagui/geometry/box.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/geometry/rot.hpp"
#include <vector>

namespace datagui {

class MeshShader {
public:
  void init();

  void queue_mesh(
      const Mesh& mesh,
      const Vec3& position,
      const Rot3& orientation,
      const Color& color);

  void draw(const Box2& viewport, const Camera3d& camera);
  void clear();

private:
  struct Command {
    Mesh mesh;
    Mat4 model_mat;
    Color color;
  };
  std::vector<Command> commands;

  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_P;
  unsigned int uniform_V;
  unsigned int uniform_M;
  unsigned int uniform_mesh_color;
};

} // namespace datagui
