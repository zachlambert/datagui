#pragma once

#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/shape_3d_shader.hpp"

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

  void grid(std::size_t size, float width);

private:
  void begin() override;
  void end() override;
  void impl_init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontManager>& fm) override;
  void mouse_event(const Vec2& size, const MouseEvent& event) override;
  bool scroll_event(const Vec2& size, const ScrollEvent& event) override;

  Camera3d camera;
  Shape3dShader shape_shader;

  Vec3 click_point;
  Vec3 click_point_direction;
  Vec3 click_camera_direction;
  float click_distance_z;
};

} // namespace datagui
