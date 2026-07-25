#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/render/glyph_2d_vertex.hpp"

namespace dgui {

class FontProgram {
public:
  void init();

  void bind();

  void draw_bitmap(
      const Box2& box,
      int bitmap_width,
      int bitmap_height,
      const void* bitmap_data);

private:
  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_PV;
  unsigned int uniform_text_color;

  // Array/buffer objects
  unsigned int VAO, VBO;
};

} // namespace dgui
