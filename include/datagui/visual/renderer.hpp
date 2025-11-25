#pragma once

#include "datagui/visual/box_shader.hpp"
#include "datagui/visual/font_manager.hpp"
#include "datagui/visual/text_shader.hpp"
#include <assert.h>
#include <memory>
#include <stack>

namespace datagui {

class Renderer {
public:
  void init(std::shared_ptr<FontManager> fm) {
    box_shader.init();
    text_shader.init(fm);
  }

  void queue_box(
      const Box2& box,
      const Color& bg_color,
      float border_width = 0,
      Color border_color = Color::Black()) {
    assert(!masks.empty());
    box_shader.queue_box(
        box,
        bg_color,
        border_width,
        border_color,
        z_index++,
        masks.top());
  }

  void queue_text(
      const Vec2& origin,
      const std::string& text,
      Font font,
      int font_size,
      Color text_color,
      Length width = LengthWrap()) {
    assert(!masks.empty());
    text_shader.queue_text(
        origin,
        text,
        font,
        font_size,
        text_color,
        width,
        z_index++,
        masks.top());
  }

  void render_begin(const Vec2& viewport_size) {
    this->viewport_size = viewport_size;
    if (masks.size() == 1) {
      masks.pop();
    }
    masks.push(Box2(Vec2(), viewport_size));
    z_index = 0;
  }

  void render() {
    box_shader.render(viewport_size, z_index);
    text_shader.render(viewport_size, z_index);
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
  BoxShader box_shader;
  TextShader text_shader;
  Vec2 viewport_size;
  std::stack<Box2> masks;
  int z_index;
};

} // namespace datagui
