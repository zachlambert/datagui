#pragma once

#include "datagui/widget/widget.hpp"
#include "datagui/render/state/scene_3d.hpp"
#include <functional>

namespace dgui {

class Canvas3d : public Widget {
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

  // A mesh loaded with Mesh::load_textured() is drawn with its texture, in
  // which case color acts as a tint (use the alpha channel for opacity)
  void mesh(
      const Mesh& mesh,
      const Vec3& position,
      const Rot3& orientation,
      const Color& color = Color::White());

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

  Vec2 min_size() const override { return Vec2::uniform(border_width_ * 2); }
  void set_dependent_state(const Box2& box) override;
  void render(const Box2& box, DrawList& dl) const override;

  void mouse_event(const Box2& box, const MouseEvent& event) override;
  bool scroll_event(const Box2& box, const ScrollEvent& event) override;
  void reset_camera();

  static constexpr float border_width_ = 2;

  Camera3d camera;
  std::shared_ptr<Scene3d> scene;

  float aspect_ratio_ = 1;
  Camera3d click_camera;
  std::function<void(const MouseEvent& event)> click_callback_;
};

} // namespace dgui
