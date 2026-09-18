#pragma once

#include "datagui/asset/mesh.hpp"
#include "datagui/asset/point_cloud.hpp"
#include "datagui/render/state/shape_3d_instance.hpp"

namespace dgui {

struct Scene3d {
  struct MeshInstance {
    Mesh mesh;
    Mat4 transform;
    Color base_color;
  };
  struct PointCloudInstance {
    PointCloud point_cloud;
    Mat4 transform;
    float point_size;
    Color base_color;
  };

  Color bg_color = Color::White();
  std::array<std::vector<Shape3dInstance>, Shape3dTypeCount> shape_instances;
  std::vector<MeshInstance> mesh_instances;
  std::vector<PointCloudInstance> point_cloud_instances;

  void clear();

  void draw_box(
      const Vec3& position,
      const Rot3& orientation,
      const Vec3& scale,
      const Color& color);

  void draw_cylinder(
      const Vec3& base_position,
      const Vec3& direction,
      float radius,
      float length,
      const Color& color);

  void draw_sphere(const Vec3& position, float radius, const Color& color);

  void draw_half_sphere(
      const Vec3& position,
      const Vec3& direction,
      float radius,
      const Color& color);

  void draw_cone(
      const Vec3& base_position,
      const Vec3& direction,
      float radius,
      float length,
      const Color& color);

  void draw_plane(
      const Vec3& position,
      const Rot3& orientation,
      const Vec2& scale,
      const Color& color);

  void draw_capsule(
      const Vec3& start,
      const Vec3& end,
      float radius,
      const Color& color);

  void draw_arrow(
      const Vec3& start,
      const Vec3& end,
      float radius,
      const Color& color,
      float head_length_scale = 2,
      float head_radius_scale = 2);

  void draw_mesh(
      const Mesh& mesh,
      const Vec3& position,
      const Rot3& rotation,
      const Vec3& scale,
      const Color& base_color = Color::White());

  void draw_point_cloud(
      const PointCloud& point_cloud,
      const Vec3& position,
      const Rot3& rotation,
      const Vec3& scale,
      float point_size,
      const Color& base_color = Color::White());

private:
  void draw_shape(Shape3dType type, const Shape3dInstance& instance);
};

} // namespace dgui
