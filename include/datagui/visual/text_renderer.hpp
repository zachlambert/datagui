#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/visual/font.hpp"
#include <string>
#include <vector>

namespace datagui {

class TextRenderer {
public:
  TextRenderer(FontManager& font_manager) : font_manager(font_manager) {}
  void init();

  void queue_text(
      const std::string& text,
      const Vecf& origin,
      const Font font,
      int font_size,
      const Color& font_color,
      float max_width);

  void render(const Vecf& viewport_size);

private:
  struct Vertex {
    Vecf pos;
    Vecf uv;
  };

  struct Command {
    const unsigned int font_texture;
    const Color font_color;
    std::vector<Vertex> vertices;

    Command(unsigned int font_texture, const Color& font_color) :
        font_texture(font_texture), font_color(font_color) {}
  };

  FontManager& font_manager;
  std::vector<Command> commands;

  struct {
    // Shader
    unsigned int program_id;
    // Uniforms
    unsigned int uniform_viewport_size;
    unsigned int uniform_text_color;
    // Array/buffer objects
    unsigned int VAO, VBO;
  } gl_data;
};

} // namespace datagui
