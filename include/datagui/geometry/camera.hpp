#pragma once

#include "datagui/geometry/mat.hpp"

namespace datagui {

struct Camera3d {
  Vec3 position;
  Vec3 direction;
  float clipping_min;
  float clipping_max;
  float fov_horizontal_degrees;
  float fov_vertical_degrees;

  Mat4 view_mat() const {
    Mat4 view;
    view(3, 3) = 1;
    for (std::size_t i = 0; i < 3; i++) {
      view(i, 3) = position(i);
    }
    const Vec3& n3 = direction;
    Vec3 n2 = n3.cross(Vec3(0, 0, 1));
    n2 /= n2.length();
    Vec3 n1 = n3.cross(n2);
    for (std::size_t i = 0; i < 3; i++) {
      view(i, 0) = n1(i);
      view(i, 1) = n2(i);
      view(i, 2) = n3(i);
    }
    return view.transpose();
  }

  Mat4 projection() const {
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
};

} // namespace datagui
