#pragma once

#include "datagui/asset/point_cloud.hpp"
#include "datagui/color.hpp"
#include "datagui/geometry/mat.hpp"

namespace dgui {

class PointCloudProgram {
public:
  void init();
  void bind();

  void draw(
      const Mat4& view,
      const Mat4& projection,
      const PointCloud& point_cloud,
      const Mat4& transform,
      float point_size,
      const Color& base_color = Color::White());

private:
  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_P;
  unsigned int uniform_V;
  unsigned int uniform_M;
  unsigned int uniform_point_size;
};

} // namespace dgui
