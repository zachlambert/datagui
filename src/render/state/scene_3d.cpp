#include "datagui/render/state/scene_3d.hpp"

namespace dgui {

void Scene3d::clear() {
  bg_color = Color::White();
  for (size_t i = 0; i < size_t(Shape3dTypeCount); i++) {
    shape_instances[i].clear();
  }
  mesh_instances.clear();
  point_cloud_instances.clear();
}

void Scene3d::draw_box(
    const Vec3& position,
    const Rot3& orientation,
    const Vec3& scale,
    const Color& color) {
  draw_shape(
      Shape3dType::Box,
      Shape3dInstance::box(position, orientation, scale, color));
}

void Scene3d::draw_cylinder(
    const Vec3& base_position,
    const Vec3& direction,
    float radius,
    float length,
    const Color& color) {
  draw_shape(
      Shape3dType::Cylinder,
      Shape3dInstance::cylinder(
          base_position,
          direction,
          radius,
          length,
          color));
}

void Scene3d::draw_sphere(
    const Vec3& position,
    float radius,
    const Color& color) {
  draw_shape(
      Shape3dType::Sphere,
      Shape3dInstance::sphere(position, radius, color));
}

void Scene3d::draw_half_sphere(
    const Vec3& position,
    const Vec3& direction,
    float radius,
    const Color& color) {
  draw_shape(
      Shape3dType::HalfSphere,
      Shape3dInstance::half_sphere(position, direction, radius, color));
}

void Scene3d::draw_cone(
    const Vec3& base_position,
    const Vec3& direction,
    float radius,
    float length,
    const Color& color) {
  draw_shape(
      Shape3dType::Cone,
      Shape3dInstance::cone(base_position, direction, radius, length, color));
}

void Scene3d::draw_plane(
    const Vec3& position,
    const Rot3& orientation,
    const Vec2& scale,
    const Color& color) {
  draw_shape(
      Shape3dType::Plane,
      Shape3dInstance::plane(position, orientation, scale, color));
}

void Scene3d::draw_capsule(
    const Vec3& start,
    const Vec3& end,
    float radius,
    const Color& color) {
  auto capsule = Capsule3dInstance::make(start, end, radius, color);
  draw_shape(Shape3dType::Cylinder, capsule.cylinder);
  draw_shape(Shape3dType::HalfSphere, capsule.half_sphere_1);
  draw_shape(Shape3dType::HalfSphere, capsule.half_sphere_2);
}

void Scene3d::draw_arrow(
    const Vec3& start,
    const Vec3& end,
    float radius,
    const Color& color,
    float head_length_scale,
    float head_radius_scale) {
  auto arrow = Arrow3dInstance::make(
      start,
      end,
      radius,
      color,
      head_length_scale,
      head_radius_scale);
  draw_shape(Shape3dType::Cylinder, arrow.cylinder);
  draw_shape(Shape3dType::Cone, arrow.cone);
}

void Scene3d::draw_shape(Shape3dType type, const Shape3dInstance& instance) {
  shape_instances[size_t(type)].push_back(instance);
}

void Scene3d::draw_mesh(
    const Mesh& mesh,
    const Vec3& position,
    const Rot3& rotation,
    const Vec3& scale,
    const Color& base_color) {
  mesh_instances.push_back(
      MeshInstance{
          mesh,
          Mat4::transform(position, rotation, scale),
          base_color});
}

void Scene3d::draw_point_cloud(
    const PointCloud& point_cloud,
    const Vec3& position,
    const Rot3& rotation,
    const Vec3& scale,
    float point_size,
    const Color& base_color) {
  point_cloud_instances.push_back(
      PointCloudInstance{
          point_cloud,
          Mat4::transform(position, rotation, scale),
          point_size,
          base_color});
}

} // namespace dgui
