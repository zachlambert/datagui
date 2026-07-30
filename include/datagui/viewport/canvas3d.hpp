#pragma once

#include "datagui/viewport/viewport.hpp"
#include "datagui/asset/uv_mesh.hpp"
#include "datagui/render/state/scene_3d.hpp"
#include <functional>

namespace dgui {

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
    scene->bg_color = color;
  }

  void aspect_ratio(float aspect_ratio) {
    aspect_ratio_ = aspect_ratio;
  }

  void click_callback(const std::function<void(const MouseEvent&)>& callback) {
    click_callback_ = callback;
  }

private:
  void init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontRegistry>& font_registry) override;
  void begin() override;
  void draw(const Box2& viewport, DrawList& dl) override;

  void mouse_event(const MouseEvent& event) override;
  bool scroll_event(const ScrollEvent& event) override;
  void reset_camera();

  Camera3d camera;
  std::shared_ptr<Scene3d> scene;

  float aspect_ratio_ = 1;
  Camera3d click_camera;
  std::function<void(const MouseEvent& event)> click_callback_;
};

} // namespace dgui
