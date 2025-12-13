#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/visual/font_manager.hpp"
#include <memory>
#include <string>
#include <vector>

namespace datagui {

class TextShader {
  struct Vertex {
    Vec2 pos;
    Vec2 uv;
  };

  struct CharList {
    const unsigned int font_texture;
    const Color font_color;
    std::vector<Vertex> vertices;

    CharList(unsigned int font_texture, const Color& font_color) :
        font_texture(font_texture), font_color(font_color) {}
  };

public:
  void init(const std::shared_ptr<FontManager>& fm);

  void queue_text(
      const Vec2& origin,
      const std::string& text,
      Font font,
      int font_size,
      Color text_color,
      Length width,
      const Box2& mask);

  void queue_text(
      const Vec2& origin,
      float angle,
      const std::string& text,
      Font font,
      int font_size,
      Color text_color,
      Length width);

  void draw(const Vec2& viewport_size);
  void clear();

private:
  void queue_text(
      const Vec2& origin,
      float angle,
      const std::string& text,
      Font font,
      int font_size,
      Color text_color,
      Length width,
      const std::optional<Box2>& mask);

  std::shared_ptr<FontManager> fm;
  std::vector<CharList> char_lists;

  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_y_dir;
  unsigned int uniform_viewport_size;
  unsigned int uniform_text_color;

  // Array/buffer objects
  unsigned int VAO, VBO;
};

} // namespace datagui
