#include "datagui/render/state/shape_3d_instance.hpp"
#include "datagui/geometry/rot.hpp"
#include <algorithm>

namespace dgui {

Shape3dInstance Shape3dInstance::box(
    const Vec3& position,
    const Rot3& orientation,
    const Vec3& scale,
    const Color& color) {
  return {Mat4::transform(position, orientation, scale), color};
}

Shape3dInstance Shape3dInstance::cylinder(
    const Vec3& base_position,
    const Vec3& direction,
    float radius,
    float length,
    const Color& color) {
  return {
      Mat4::transform(
          base_position,
          Rot3::line_rot(direction),
          Vec3(length, radius, radius)),
      color};
}

Shape3dInstance Shape3dInstance::sphere(
    const Vec3& position,
    float radius,
    const Color& color) {
  return {Mat4::transform(position, Rot3(), Vec3::uniform(radius)), color};
}

Shape3dInstance Shape3dInstance::half_sphere(
    const Vec3& position,
    const Vec3& direction,
    float radius,
    const Color& color) {
  return {
      Mat4::transform(
          position,
          Rot3::line_rot(direction),
          Vec3::uniform(radius)),
      color};
}

Shape3dInstance Shape3dInstance::cone(
    const Vec3& base_position,
    const Vec3& direction,
    float radius,
    float length,
    const Color& color) {
  return {
      Mat4::transform(
          base_position,
          Rot3::line_rot(direction),
          Vec3(length, radius, radius)),
      color};
}

Shape3dInstance Shape3dInstance::plane(
    const Vec3& position,
    const Rot3& orientation,
    const Vec2& scale,
    const Color& color) {
  return {
      Mat4::transform(position, orientation, Vec3(scale.x, scale.y, 1)),
      color};
}

Capsule3dInstance Capsule3dInstance::make(
    const Vec3& start,
    const Vec3& end,
    float radius,
    const Color& color) {
  float length = (end - start).length();
  Vec3 direction = length > 0 ? (end - start) / length : Vec3(1, 0, 0);

  Capsule3dInstance capsule;
  capsule.cylinder =
      Shape3dInstance::cylinder(start, direction, radius, length, color);
  capsule.half_sphere_1 =
      Shape3dInstance::half_sphere(start, -direction, radius, color);
  capsule.half_sphere_2 =
      Shape3dInstance::half_sphere(end, direction, radius, color);
  return capsule;
}

Arrow3dInstance Arrow3dInstance::make(
    const Vec3& start,
    const Vec3& end,
    float radius,
    const Color& color,
    float head_length_scale,
    float head_radius_scale) {
  head_length_scale = std::max(head_length_scale, 1.f);
  head_radius_scale = std::max(head_radius_scale, 1.f);

  float length = (end - start).length();
  Vec3 direction = length > 0 ? (end - start) / length : Vec3(1, 0, 0);
  float head_length = std::min(length, head_length_scale * 2 * radius);
  float line_length = std::max(length - head_length, 0.f);

  Arrow3dInstance arrow;
  arrow.cylinder =
      Shape3dInstance::cylinder(start, direction, radius, line_length, color),
  arrow.cone = Shape3dInstance::cone(
      start + direction * line_length,
      direction,
      head_radius_scale * radius,
      head_length,
      color);
  return arrow;
}

} // namespace dgui
