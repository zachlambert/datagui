#pragma once

#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/shape_2d_shader.hpp"
#include "datagui/visual/text_shader.hpp"

namespace datagui {

class Canvas2 : public Viewport {
public:
  void box(
      const Box2& box,
      const Color& color,
      float radius = 0,
      float border_width = 0,
      const Color& border_color = Color::Black());

  void queue_text(
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
  void mouse_event(const Vec2& size, const MouseEvent& event) override;

  Shape2dShader shape_shader;
  TextShader text_shader;
};

} // namespace datagui
