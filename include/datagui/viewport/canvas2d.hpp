#pragma once

#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/image_shader.hpp"
#include "datagui/visual/shape_2d_shader.hpp"
#include "datagui/visual/text_2d_shader.hpp"
#include <functional>
#include <optional>

namespace dgui {

class Canvas2d : public Viewport {
public:
  void rect(
      const Vec2& position,
      float angle,
      const Vec2& size,
      const Color& color,
      float border_width = 0,
      const Color& border_color = Color::Black());

  void circle(
      const Vec2& position,
      float radius,
      const Color& color,
      float border_width = 0,
      const Color& border_color = Color::Black());

  void ellipse(
      const Vec2& position,
      float angle,
      const Vec2& radii,
      const Color& color,
      float border_width = 0,
      const Color& border_color = Color::Black());

  void line(const Vec2& a, const Vec2& b, float width, const Color& color);

  void capsule(
      const Vec2& a,
      const Vec2& b,
      float radius,
      const Color& color,
      float border_width = 0,
      const Color& border_color = Color::Black());

  void text(
      const std::string& text,
      const Vec2& origin,
      float angle = 0,
      int font_size = 20,
      Font font = Font::DejaVuSans,
      Color text_color = Color::Black(),
      Length width = LengthWrap());

  void heatmap(
      const Vec2& lower,
      const Vec2& upper,
      const std::function<float(float x, float y)>& function,
      float min_value,
      float max_value,
      std::size_t width = 256,
      std::size_t height = 256);

  void default_view_position(const Vec2& position) {
    default_position_ = position;
  }
  void default_view_width(float view_width) {
    default_view_width_ = view_width;
  }

  void bg_color(const Color& color) {
    bg_color_ = color;
  }

  std::optional<MouseEvent> mouse_event();

private:
  void init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontManager>& fm) override;
  void begin() override;
  void draw(const Box2& viewport, const Box2& mask) override;

  void mouse_event(const MouseEvent& event) override;
  bool scroll_event(const ScrollEvent& event) override;

  Color bg_color_ = Color::Gray(0.95);
  Camera2d click_camera;
  std::optional<MouseEvent> mouse_event_;

  Vec2 default_position_;
  float default_view_width_;

  float zoom = 1;
  std::optional<Box2> prev_viewport_;

  Camera2d camera;
  Shape2dShader bg_shader;
  Shape2dShader shape_shader;
  Text2dShader text_shader;
  ImageShader image_shader;
};

} // namespace dgui
