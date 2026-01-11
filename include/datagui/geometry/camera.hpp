#pragma once

#include "datagui/geometry/mat.hpp"
#include "datagui/geometry/rot.hpp"

namespace datagui {

/*
 * The to_camera and from_camera methods convert to/from
 * "global" coordinates and camera coordinates.
 * For these functions, the camera coordinate are defined
 * as being normalized to the range [0, 1] within the
 * camera view box/frustum.
 *
 * This makes the logic simpler when working with Box2.to_normalized()
 * which also uses the range [0, 1]
 *
 * However, opengl expects the camera coordinates to be normalized
 * to the range [-1, 1] instead.
 * Therefore, the projection_mat() methods return the projection matrix
 * for the range [-1, 1] instead, which is not consistent with the
 * to_camera() / from_camera() methods.
 *
 */

struct Camera2d {
  Vec2 position;
  float angle = 0;
  Vec2 size;

  Mat3 view_mat() const;
  Mat3 projection_mat() const;

  Vec2 to_camera(const Vec2& world_pos) const;
  Vec2 from_camera(const Vec2& camera_pos) const;
};

struct Camera3d {
  Vec3 position;
  Vec3 direction;
  float clipping_min;
  float clipping_max;
  Vec2 fov;

  Rot3 rotation() const;
  Mat4 view_mat() const;
  Mat4 projection_mat() const;

  Vec2 to_camera(const Vec3& world_pos) const;
  Vec3 ray_from_camera(const Vec2& camera_pos) const;
};

} // namespace datagui
