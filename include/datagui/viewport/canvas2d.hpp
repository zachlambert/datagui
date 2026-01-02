#pragma once

#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/image_shader.hpp"
#include "datagui/visual/shape_2d_shader.hpp"
#include "datagui/visual/text_2d_shader.hpp"
#include <functional>

namespace datagui {

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

  void scale(float scale) {
    scale_ = scale;
  }
  void bg_color(const Color& color) {
    bg_color_ = color;
  }

private:
  void begin() override;
  void end() override;

  void impl_init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontManager>& fm) override;
  void redraw();

  void mouse_event(const Vec2& size, const MouseEvent& event) override;
  bool scroll_event(const Vec2& size, const ScrollEvent& event) override;

  std::optional<float> scale_;
  Color bg_color_ = Color::Gray(0.95);
  Vec2 click_camera_pos;
  Vec2 click_mouse_pos;

  Camera2d camera;
  Shape2dShader shape_shader;
  Text2dShader text_shader;
  ImageShader image_shader;
};

} // namespace datagui
