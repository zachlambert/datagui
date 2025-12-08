#include "datagui/visual/gui_renderer.hpp"
#include <GLFW/glfw3.h>
#include <assert.h>
#include <memory>
#include <stack>

namespace datagui {

void GuiRenderer::init(std::shared_ptr<FontManager> fm) {
  shape_shader.init();
  text_shader.init();
  image_shader.init();
  this->fm = fm;
}

void GuiRenderer::queue_box(
    const Box2& box,
    const Color& bg_color,
    float border_width,
    Color border_color) {
  assert(!layers.empty());
  assert(!masks.empty());
  layers.back().shape_command.queue_box(
      flip_box(box),
      bg_color,
      0,
      border_width,
      border_color,
      masks.top());
}

void GuiRenderer::queue_text(
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
      flip_position(origin, font, font_size),
      text,
      font,
      font_size,
      text_color,
      width,
      masks.top());
}

void GuiRenderer::queue_image(const Box2& box, unsigned int texture) {
  layers.back().image_commands.push_back(
      {flip_box(box), texture, OpenglRgbImage()});
}

void GuiRenderer::queue_image(
    const Box2& box,
    std::size_t width,
    std::size_t height,
    void* pixels) {
  OpenglRgbImage image;
  image.write(width, height, pixels);
  layers.back().image_commands.push_back({flip_box(box), 0, std::move(image)});
}

void GuiRenderer::render_begin(const Vec2& viewport_size) {
  this->viewport_size = viewport_size;
  if (masks.size() == 1) {
    masks.pop();
  }
  masks.push(Box2(Vec2(), viewport_size));
  layers.emplace_back();
}

void GuiRenderer::render_end() {
  assert(masks.size() == 1);
  masks.pop();

  for (const auto& layer : layers) {
    shape_shader.draw(layer.shape_command, viewport_size);
    text_shader.draw(layer.text_command, viewport_size);
    for (const auto& image : layer.image_commands) {
      if (image.texture > 0) {
        image_shader.draw(image.texture, image.box, viewport_size);
      } else if (image.image.texture() > 0) {
        image_shader.draw(image.image.texture(), image.box, viewport_size);
      }
    }
  }
  layers.clear();
}

void GuiRenderer::new_layer() {
  layers.emplace_back();
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
  result.lower.y = viewport_size.y - box.upper.y;
  result.upper.y = viewport_size.y - box.lower.y;
  return result;
}

Vec2 GuiRenderer::flip_position(const Vec2& origin, Font font, int font_size) {
  return Vec2(origin.x, viewport_size.y - origin.y);
}

} // namespace datagui
