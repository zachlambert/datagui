#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/render/state/glyph_2d_instance.hpp"

namespace dgui {

class Glyph2dProgram {
public:
  void init();
  void bind();

  void draw(
      unsigned int font_texture,
      const Color& color,
      const Glyph2dInstance* data,
      size_t count,
      const Mat3& PV);

private:
  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_PV;
  unsigned int uniform_text_color;

  // Array/buffer objects
  unsigned int VAO;
  unsigned int static_VBO;
  unsigned int instance_VBO;
  std::size_t static_vertex_count = 0;
};

} // namespace dgui
