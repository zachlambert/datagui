#pragma once

#include "datagui/asset/uv_mesh.hpp"
#include "datagui/geometry/mat.hpp"

namespace dgui {

class UvMeshProgram {
public:
  void init();
  void bind();

  void draw(
      const UvMesh& uv_mesh,
      const Mat4& model,
      float opacity,
      const Mat4& P,
      const Mat4& V);

private:
  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_P;
  unsigned int uniform_V;
  unsigned int uniform_M;
  unsigned int uniform_opacity;
};

} // namespace dgui
