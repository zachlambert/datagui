#include "datagui/geometry/camera.hpp"

namespace datagui {

Mat3 Camera2d::view_mat() const {
  Rot2 R_T = Rot2(angle).mat().transpose();
  Vec2 minus_R_T_pos = R_T.mat() * (-position);

  Mat3 view;
  view(2, 2) = 1;
  for (std::size_t i = 0; i < 2; i++) {
    for (std::size_t j = 0; j < 2; j++) {
      view(i, j) = R_T.mat()(i, j);
    }
  }
  for (std::size_t i = 0; i < 2; i++) {
    view(i, 2) = minus_R_T_pos(i);
  }
  return view;
}

Mat3 Camera2d::projection_mat() const {
  return Mat3{
      {1 / (0.5f * size.x), 0, 0},
      {0, 1 / (0.5f * size.y), 0},
      {0, 0, 1.f},
  };
}

Vec2 Camera2d::to_camera(const Vec2& world_pos) const {
  Vec2 pos_cs = Rot2(-angle) * (world_pos - position);
  return Vec2::uniform(0.5) + pos_cs / size;
}

Vec2 Camera2d::from_camera(const Vec2& camera_pos) const {
  Vec2 pos_cs = (camera_pos - Vec2::uniform(0.5)) * size;
  return position + Rot2(angle) * pos_cs;
}

Rot3 Camera3d::rotation() const {
  const Vec3& n3 = -direction;
  Vec3 n1 = Vec3(0, 0, 1).cross(n3);
  n1 /= n1.length();
  Vec3 n2 = n3.cross(n1);
  return Rot3(Mat3(n1, n2, n3));
}

Mat4 Camera3d::view_mat() const {
  Mat3 R_T = rotation().mat().transpose();
  Vec3 minus_R_T_pos = R_T * (-position);

  Mat4 view;
  view(3, 3) = 1;
  for (std::size_t i = 0; i < 3; i++) {
    view(i, 3) = minus_R_T_pos(i);
  }
  for (std::size_t i = 0; i < 3; i++) {
    for (std::size_t j = 0; j < 3; j++) {
      view(i, j) = R_T(i, j);
    }
  }

  return view;
}

Mat4 Camera3d::projection_mat() const {
  Mat4 projection;

  projection(0, 0) = 1.f / std::tan(0.5f * fov.x);
  projection(1, 1) = 1.f / std::tan(0.5f * fov.y);
  projection(2, 2) =
      -(clipping_min + clipping_max) / (clipping_max - clipping_min);
  projection(2, 3) =
      -2 * clipping_min * clipping_max / (clipping_max - clipping_min);
  projection(3, 2) = -1;

  return projection;
}

Vec2 Camera3d::to_camera(const Vec3& world_pos) const {
  Vec3 pos_cs = rotation().inverse() * (world_pos - position);
  Vec2 view_frustum = Vec2(std::tan(0.5f * fov.x), std::tan(0.5f * fov.y));
  return Vec2::uniform(0.5) +
         0.5 * (Vec2(pos_cs.x, pos_cs.y) / pos_cs.z) / view_frustum;
}

Vec3 Camera3d::ray_from_camera(const Vec2& camera_pos) const {
  Vec2 view_frustum = Vec2(std::tan(0.5f * fov.x), std::tan(0.5f * fov.y));
  Vec2 pos_cs = (camera_pos - Vec2::uniform(0.5)) * view_frustum;
  Vec3 ray_cs = Vec3(pos_cs.x, pos_cs.y, 1);
  return rotation() * ray_cs / ray_cs.length();
}

} // namespace datagui
