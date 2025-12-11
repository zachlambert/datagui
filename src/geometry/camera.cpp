#include "datagui/geometry/camera.hpp"

namespace datagui {

Mat4 Camera3d::view_mat() const {
  const Vec3& n3 = direction;
  Vec3 n2 = n3.cross(Vec3(0, 0, 1));
  n2 /= n2.length();
  Vec3 n1 = n3.cross(n2);

  Mat3 R_T = Mat3(n1, n2, n3).transpose();
  Vec3 R_T_pos = R_T * position;

  Mat4 view;
  view(3, 3) = 1;
  for (std::size_t i = 0; i < 3; i++) {
    view(i, 3) = R_T_pos(i);
  }
  for (std::size_t i = 0; i < 3; i++) {
    for (std::size_t j = 0; j < 3; i++) {
      view(i, j) = R_T(i, j);
    }
  }

  return view;
}

Mat4 Camera3d::projection_mat() const {
  Mat4 projection;

  float half_width =
      clipping_min * std::tan(fov_horizontal_degrees * M_PI / 180.f);
  float half_height =
      clipping_min * std::tan(fov_vertical_degrees * M_PI / 180.f);
  projection(0, 0) = clipping_min / half_width;
  projection(1, 1) = clipping_min / half_height;
  projection(2, 2) =
      -(clipping_min + clipping_max) / (clipping_max - clipping_min);
  projection(2, 3) =
      -2 * clipping_min * clipping_max / (clipping_max - clipping_min);
  projection(3, 2) = -1;

  return projection;
}

} // namespace datagui
