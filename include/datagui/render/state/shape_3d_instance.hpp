#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry/mat.hpp"
#include "datagui/geometry/vec.hpp"

namespace dgui {

enum class Shape3dType {
  Box,
  Cylinder,
  Sphere,
  HalfSphere,
  Cone,
  Plane,
};
static constexpr std::size_t Shape3dTypeCount = 6;

class Rot3;

struct Shape3dInstance {
  Mat4 transform;
  Color color;

  static Shape3dInstance box(
      const Vec3& position,
      const Rot3& orientation,
      const Vec3& scale,
      const Color& color);

  static Shape3dInstance cylinder(
      const Vec3& base_position,
      const Vec3& direction,
      float radius,
      float length,
      const Color& color);

  static Shape3dInstance sphere(
      const Vec3& position,
      float radius,
      const Color& color);

  static Shape3dInstance half_sphere(
      const Vec3& position,
      const Vec3& direction,
      float radius,
      const Color& color);

  static Shape3dInstance cone(
      const Vec3& base_position,
      const Vec3& direction,
      float radius,
      float length,
      const Color& color);

  static Shape3dInstance plane(
      const Vec3& position,
      const Rot3& orientation,
      const Vec2& scale,
      const Color& color);
};

struct CapsuleInstance {
  Shape3dInstance half_sphere_1;
  Shape3dInstance half_sphere_2;
  Shape3dInstance cylinder;

  static CapsuleInstance make(
      const Vec3& start,
      const Vec3& end,
      float radius,
      const Color& color);
};

struct ArrowInstance {
  Shape3dInstance cylinder;
  Shape3dInstance cone;

  static ArrowInstance make(
      const Vec3& start,
      const Vec3& end,
      float radius,
      const Color& color,
      float head_length_scale = 2,
      float head_radius_scale = 2);
};

} // namespace dgui
