#pragma once

#include "datagui/geometry/mat.hpp"
#include "datagui/geometry/rot.hpp"

namespace datagui {

struct Camera2d {
  Vec2 position;
  float angle;
  float width;

  Mat3 view_mat() const;
  Mat3 projection_mat(const Vec2& viewport_size) const;
};

struct Camera3d {
  Vec3 position;
  Vec3 direction;
  float clipping_min;
  float clipping_max;
  float fov_degrees; // Horizontal field of view for configured viewport width

  Rot3 rotation() const;
  Mat4 view_mat() const;
  Mat4 projection_mat(const Vec2& viewport_size) const;
};

} // namespace datagui
