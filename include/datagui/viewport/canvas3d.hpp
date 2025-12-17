#pragma once

#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/mesh_shader.hpp"
#include "datagui/visual/point_cloud_shader.hpp"
#include "datagui/visual/shape_3d_shader.hpp"
#include "datagui/visual/uv_mesh_shader.hpp"

namespace datagui {

class Canvas3d : public Viewport {
public:
  Canvas3d();

  void box(
      const Vec3& position,
      const Rot3& orientation,
      const Vec3& size,
      const Color& color);

  void cylinder(
      const Vec3& base_position,
      const Vec3& direction,
      float radius,
      float length,
      const Color& color);

  void sphere(const Vec3& position, float radius, const Color& color);

  void cone(
      const Vec3& base_position,
      const Vec3& direction,
      float radius,
      float length,
      const Color& color);

  void capsule(
      const Vec3& start,
      const Vec3& end,
      float radius,
      const Color& color);

  void arrow(
      const Vec3& start,
      const Vec3& end,
      float radius,
      const Color& color,
      float head_length_scale = 2,
      float head_radius_scale = 2);

  void plane(
      const Vec3& position,
      const Rot3& orientation,
      const Vec2& scale,
      const Color& color);

  void axes(
      const Vec3& position,
      const Rot3& orientation,
      float scale = 1,
      float line_radius = 0.05,
      float head_length_scale = 2,
      float head_radius_scale = 2);

  void grid(std::size_t size, float width);

  void mesh(
      const Mesh& mesh,
      const Vec3& position,
      const Rot3& orientation,
      const Color& color);

  void uv_mesh(
      const UvMesh& uv_mesh,
      const Vec3& position,
      const Rot3& orientation,
      float opacity = 1);

  void point_cloud(
      const PointCloud& point_cloud,
      const Vec3& position,
      const Rot3& orientation,
      float point_size);

  void bg_color(const Color& color) {
    bg_color_ = color;
  }

private:
  void begin() override;
  void end() override;
  void redraw();

  void impl_init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontManager>& fm) override;
  void mouse_event(const Vec2& size, const MouseEvent& event) override;
  bool scroll_event(const Vec2& size, const ScrollEvent& event) override;

  Camera3d camera;
  Shape3dShader shape_shader;
  MeshShader mesh_shader;
  UvMeshShader uv_mesh_shader;
  PointCloudShader point_cloud_shader;

  Color bg_color_ = Color::White();
  Vec3 click_point;
  Vec3 click_point_direction;
  Vec3 click_camera_direction;
  float click_distance_z;
};

} // namespace datagui
