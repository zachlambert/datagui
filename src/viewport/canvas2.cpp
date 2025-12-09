#include "datagui/viewport/canvas2.hpp"

namespace datagui {

void Canvas2::box(
    const Box2& box,
    const Color& color,
    float radius,
    float border_width,
    const Color& border_color,
    int z_index) {
  auto& layer = get_layer(z_index);
  layer.shape_command.queue_box(
      box,
      color,
      radius,
      border_width,
      border_color,
      Box2(Vec2(), framebuffer_size()));
}

void Canvas2::begin() {
  layers.clear();
}

void Canvas2::end() {
  render_content();
}

void Canvas2::mouse_event(const MouseEvent& event) {}

void Canvas2::impl_init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontManager>& fm) {
  shape_shader.init();
  text_shader.init();
}

void Canvas2::impl_render() {
  for (const auto& [z_index, layer] : layers) {
    shape_shader.draw(layer.shape_command, framebuffer_size());
    text_shader.draw(layer.text_command, framebuffer_size());
  }
}

Canvas2::Layer& Canvas2::get_layer(int z_index) {
  auto iter = layers.find(z_index);
  if (iter != layers.end()) {
    return iter->second;
  }
  return layers.emplace(z_index, Layer()).first->second;
}

}; // namespace datagui
