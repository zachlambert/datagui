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
}

void Canvas2d::circle(
    const Vec2& position,
    float radius,
    const Color& color,
    float border_width,
    const Color& border_color) {
  shape_shader.queue_circle(
      position,
      radius,
      color,
      border_width,
      border_color,
      Box2(Vec2(), framebuffer_size()));
}

void Canvas2d::ellipse(
    const Vec2& position,
    float angle,
    float x_radius,
    float y_radius,
    const Color& color,
    float border_width,
    const Color& border_color) {
  shape_shader.queue_ellipse(
      position,
      angle,
      x_radius,
      y_radius,
      color,
      border_width,
      border_color,
      Box2(Vec2(), framebuffer_size()));
}

void Canvas2d::line(
    const Vec2& a,
    const Vec2& b,
    float width,
    const Color& color) {
  shape_shader.queue_line(a, b, width, color, Box2(Vec2(), framebuffer_size()));
}

void Canvas2d::capsule(
    const Vec2& a,
    const Vec2& b,
    float radius,
    const Color& color,
    float border_width,
    const Color& border_color) {
  shape_shader.queue_capsule(
      a,
      b,
      radius,
      color,
      border_width,
      border_color,
      Box2(Vec2(), framebuffer_size()));
}

void Canvas2d::text(
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
  shape_shader.clear();
  text_shader.clear();
}

void Canvas2d::end() {
  redraw();
}

void Canvas2d::impl_init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontManager>& fm) {
  shape_shader.init();
  text_shader.init(fm);
}

void Canvas2d::redraw() {
  bind_framebuffer();
  shape_shader.draw(framebuffer_size());
  text_shader.draw(framebuffer_size());
  unbind_framebuffer();
}

void Canvas2d::mouse_event(const Vec2& size, const MouseEvent& event) {
  // TODO
}

}; // namespace datagui
