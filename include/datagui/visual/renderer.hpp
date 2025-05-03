#pragma once

#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

class Renderer {
public:
  Renderer(FontManager& font_manager) : font_manager(font_manager) {}
  void init();

  void queue_text(
      const Vecf& origin,
      float z_pos,
      const std::string& text,
      Font font,
      int font_size,
      Color text_color,
      Length width);

  void queue_text(
      const Vecf& origin,
      float z_pos,
      const std::string& text,
      const TextStyle& style,
      Length width) {
    queue_text(
        origin,
        z_pos,
        text,
        style.font,
        style.font_size,
        style.text_color,
        width);
  }

  void render(const Vecf& viewport_size);

private:
  GeometryRenderer geometry_renderer;
  TextRenderer text_renderer;

  struct Layer {
    int layer;
    std::vector<GeometryRenderer::Command> geometry_commands;
    std::vector<TextRenderer::Command> text_commands;
  };
  std::map<int, Layer> layers;
};

} // namespace datagui
