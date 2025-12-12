#pragma once

#include "datagui/geometry/mat.hpp"

namespace datagui {

struct Camera3d {
  Vec3 position;
  Vec3 direction;
  float clipping_min;
  float clipping_max;
  float fov_degrees; // Horizontal field of view for configured viewport width

  Mat4 view_mat() const;
  Mat4 projection_mat(float aspect_ratio) const;
};

} // namespace datagui
