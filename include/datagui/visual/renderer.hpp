#pragma once

#include "datagui/visual/font_manager.hpp"
#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"
#include <assert.h>
#include <memory>
#include <stack>

namespace datagui {

class Renderer {
public:
  void init(std::shared_ptr<FontManager> fm) {
    geometry_renderer.init();
    text_renderer.init(fm);
  }

  void queue_box(
      const Box2& box,
      const Color& bg_color,
      BoxDims border_width,
      Color border_color,
      float radius) {
    assert(!masks.empty());
    geometry_renderer.queue_box(
        box,
        bg_color,
        border_width,
        border_color,
        radius,
        masks.top());
  }

  void queue_text(
      const Vec2& origin,
      const std::string& text,
      Font font,
      int font_size,
      Color text_color,
      Length width) {
    assert(!masks.empty());
    text_renderer.queue_text(
        origin,
        text,
        font,
        font_size,
        text_color,
        width,
        masks.top());
  }

  void render_begin(const Vec2& viewport_size) {
    this->viewport_size = viewport_size;
    if (masks.size() == 1) {
      masks.pop();
    }
    masks.push(Box2(Vec2(), viewport_size));
  }

  void render() {
    geometry_renderer.render(viewport_size);
    text_renderer.render(viewport_size);
  }

  void render_end() {
    assert(masks.size() == 1);
    masks.pop();
  }

  void push_mask(const Box2& mask) {
    if (masks.empty()) {
      masks.push(mask);
    } else {
      masks.push(intersection(masks.top(), mask));
    }
  }
  void pop_mask() {
    masks.pop();
  }

private:
  GeometryRenderer geometry_renderer;
  TextRenderer text_renderer;
  Vec2 viewport_size;
  std::stack<Box2> masks;
  int depth;
};

} // namespace datagui
