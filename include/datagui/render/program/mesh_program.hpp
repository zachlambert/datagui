#pragma once

#include "datagui/asset/mesh.hpp"
#include "datagui/color.hpp"
#include "datagui/geometry/mat.hpp"

namespace dgui {

class MeshProgram {
public:
  void init();
  void bind();

  void draw(
      const Mesh& mesh,
      const Mat4& model,
      const Color& color,
      const Mat4& P,
      const Mat4& V);

private:
  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_P;
  unsigned int uniform_V;
  unsigned int uniform_M;
  unsigned int uniform_mesh_color;
};

} // namespace dgui
