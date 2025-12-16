#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/visual/font_manager.hpp"
#include <memory>
#include <string>
#include <vector>

namespace datagui {

class Text2dShader {
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

  void queue_masked_text(
      const Box2& mask,
      const Vec2& origin,
      const std::string& text,
      Font font,
      int font_size,
      Color text_color,
      Length width = LengthWrap());

  void queue_text(
      const Vec2& origin,
      float angle,
      const std::string& text,
      Font font,
      int font_size,
      Color text_color,
      Length width = LengthWrap());

  void draw(const Box2& viewport, const Camera2d& camera);
  void clear();

private:
  std::vector<Vertex>& get_vertices(
      Font font,
      int font_size,
      const Color& color);

  std::shared_ptr<FontManager> fm;
  std::vector<CharList> char_lists;

  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_PV;
  unsigned int uniform_text_color;

  // Array/buffer objects
  unsigned int VAO, VBO;
};

} // namespace datagui
