#pragma once

#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

class Renderer {
public:
  void init(std::shared_ptr<FontManager> fm) {
    geometry_renderer.init();
    text_renderer.init(fm);
  }

  void queue_box(
      const Boxf& box,
      const Color& bg_color,
      BoxDims border_width,
      Color border_color,
      float radius) {
    geometry_renderer
        .queue_box(box, bg_color, border_width, border_color, radius);
  }

  void queue_text(
      const Vecf& origin,
      const std::string& text,
      Font font,
      int font_size,
      Color text_color,
      Length width) {
    text_renderer.queue_text(origin, text, font, font_size, text_color, width);
  }

  void queue_text(
      const Vecf& origin,
      const std::string& text,
      const TextStyle& style,
      Length width) {
    text_renderer.queue_text(
        origin,
        text,
        style.font,
        style.font_size,
        style.text_color,
        width);
  }

  void render(const Vecf& viewport_size) {
    geometry_renderer.render(viewport_size);
    text_renderer.render(viewport_size);
  }

  void push_mask(const Boxf& mask) {
    geometry_renderer.push_mask(mask);
    text_renderer.push_mask(mask);
  }
  void pop_mask() {
    geometry_renderer.pop_mask();
    text_renderer.pop_mask();
  }

private:
  GeometryRenderer geometry_renderer;
  TextRenderer text_renderer;
};

} // namespace datagui
