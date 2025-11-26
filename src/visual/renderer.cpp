#include "datagui/visual/renderer.hpp"
#include <GLFW/glfw3.h>
#include <assert.h>
#include <memory>
#include <stack>

namespace datagui {

void Renderer::init(std::shared_ptr<FontManager> fm) {
  this->fm = fm;
  shape_shader.init();
  text_shader.init();
}

void Renderer::queue_box(
    const Box2& box,
    const Color& bg_color,
    float border_width,
    Color border_color) {
  assert(!layers.empty());
  assert(!masks.empty());
  layers.back().shape_command.queue_box(
      box,
      bg_color,
      border_width,
      border_color,
      masks.top());
}

void Renderer::queue_text(
    const Vec2& origin,
    const std::string& text,
    Font font,
    int font_size,
    Color text_color,
    Length width) {
  assert(!layers.empty());
  assert(!masks.empty());
  layers.back().text_command.queue_text(
      fm,
      origin,
      text,
      font,
      font_size,
      text_color,
      width,
      masks.top());
}

void Renderer::render_begin(const Vec2& viewport_size) {
  this->viewport_size = viewport_size;
  if (masks.size() == 1) {
    masks.pop();
  }
  masks.push(Box2(Vec2(), viewport_size));
  layers.emplace_back();
}

void Renderer::render_end() {
  assert(masks.size() == 1);
  masks.pop();

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(1.f, 1.f, 1.f, 1.f);
  glClearDepth(0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (const auto& layer : layers) {
    shape_shader.draw(layer.shape_command, viewport_size);
    text_shader.draw(layer.text_command, viewport_size);
  }
  layers.clear();
}

void Renderer::new_layer() {
  layers.emplace_back();
}

void Renderer::push_mask(const Box2& mask) {
  if (masks.empty()) {
    masks.push(mask);
  } else {
    masks.push(intersection(masks.top(), mask));
  }
}

void Renderer::pop_mask() {
  masks.pop();
}

} // namespace datagui
