#pragma once

#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/shape_2d_shader.hpp"
#include "datagui/visual/text_2d_shader.hpp"

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
      float border_width,
      const Color& border_color);

  void text(
      const Vec2& origin,
      float angle,
      const std::string& text,
      Font font,
      int font_size,
      Color text_color,
      Length width);

private:
  void begin() override;
  void end() override;

  void impl_init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontManager>& fm) override;
  void redraw();

  void mouse_event(const Vec2& size, const MouseEvent& event) override;

  Camera2d camera;
  Shape2dShader shape_shader;
  Text2dShader text_shader;
};

} // namespace datagui
