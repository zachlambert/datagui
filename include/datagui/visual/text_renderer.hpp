#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/visual/font_manager.hpp"
#include <memory>
#include <string>
#include <vector>

namespace datagui {

class TextRenderer {
public:
  void init(std::shared_ptr<FontManager> fm);

  void queue_text(
      const Vec2& origin,
      const std::string& text,
      Font font,
      int font_size,
      Color text_color,
      Length width,
      const Box2& mask);

  void render(const Vec2& viewport_size);

private:
  struct Vertex {
    Vec2 pos;
    Vec2 uv;
  };

  struct Command {
    const unsigned int font_texture;
    const Color font_color;
    std::vector<Vertex> vertices;

    Command(unsigned int font_texture, const Color& font_color) :
        font_texture(font_texture), font_color(font_color) {}
  };

  std::shared_ptr<FontManager> fm;
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
