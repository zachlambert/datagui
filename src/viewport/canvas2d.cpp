#include "datagui/viewport/canvas2d.hpp"

namespace datagui {

void Canvas2d::box(
    const Box2& box,
    const Color& color,
    float radius,
    float border_width,
    const Color& border_color) {
  shape_shader.queue_box(
      box,
      color,
      radius,
      border_width,
      border_color,
      Box2(Vec2(), framebuffer_size()));
  shape_shader.draw(framebuffer_size());
}

void Canvas2d::queue_text(
    const Vec2& origin,
    float angle,
    const std::string& text,
    Font font,
    int font_size,
    Color text_color,
    Length width) {
  text_shader
      .queue_text(origin, angle, text, font, font_size, text_color, width);
  text_shader.draw(framebuffer_size());
}

void Canvas2d::begin() {
  bind_framebuffer();
}

void Canvas2d::end() {
  unbind_framebuffer();
}

void Canvas2d::impl_init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontManager>& fm) {
  shape_shader.init();
  text_shader.init(fm);
}

void Canvas2d::mouse_event(const Vec2& size, const MouseEvent& event) {
  // TODO
}

}; // namespace datagui
