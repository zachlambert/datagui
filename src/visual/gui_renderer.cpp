#include "datagui/visual/gui_renderer.hpp"
#include <GLFW/glfw3.h>
#include <assert.h>
#include <memory>
#include <stack>

namespace datagui {

void GuiRenderer::init(std::shared_ptr<FontManager> fm) {
  box_shader.init();
  text_shader.init(fm);
  image_shader.init();
  this->fm = fm;
}

void GuiRenderer::queue_box(
    const Box2& box,
    const Color& bg_color,
    float border_width,
    Color border_color,
    float radius) {
  assert(!masks.empty());
  box_shader.queue_box(
      flip_box(masks.top()),
      flip_box(box),
      bg_color,
      border_width,
      border_color,
      radius);
}

void GuiRenderer::queue_text(
    const Vec2& origin,
    const std::string& text,
    Font font,
    int font_size,
    Color text_color,
    Length width) {
  assert(!masks.empty());
  text_shader.queue_text(
      flip_position(origin),
      text,
      font,
      font_size,
      text_color,
      width,
      masks.top());
}

void GuiRenderer::queue_image(const Box2& box, const Image& image) {
  image_shader
      .queue_image(image, flip_position(box.upper_left()), 0, box.size());
}

void GuiRenderer::queue_texture(const Box2& box, int texture) {
  image_shader.queue_texture(flip_box(box), texture);
}

void GuiRenderer::begin(const Box2& viewport) {
  this->viewport = viewport;
  if (masks.size() == 1) {
    masks.pop();
  }
  masks.push(viewport);
}

void GuiRenderer::end() {
  assert(masks.size() == 1);
  masks.pop();
}

void GuiRenderer::render() {
  camera.position = viewport.center();
  camera.angle = 0;
  camera.width = viewport.size().x;
  box_shader.draw(viewport);
  text_shader.draw(viewport, camera);
  image_shader.draw(viewport, camera);
  box_shader.clear();
  text_shader.clear();
  image_shader.clear();
}

void GuiRenderer::push_mask(const Box2& mask) {
  if (masks.empty()) {
    masks.push(flip_box(mask));
  } else {
    masks.push(intersection(masks.top(), flip_box(mask)));
  }
}

void GuiRenderer::pop_mask() {
  masks.pop();
}

Box2 GuiRenderer::flip_box(const Box2& box) {
  Box2 result = box;
  result.lower.y = viewport.size().y - box.upper.y;
  result.upper.y = viewport.size().y - box.lower.y;
  return result;
}

Vec2 GuiRenderer::flip_position(const Vec2& origin) {
  return Vec2(origin.x, viewport.size().y - origin.y);
}

} // namespace datagui
