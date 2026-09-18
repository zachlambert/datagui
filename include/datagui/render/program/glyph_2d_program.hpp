#pragma once

#include "datagui/geometry/mat.hpp"
#include "datagui/render/state/glyph_2d_instance.hpp"

namespace dgui {

class Glyph2dProgram {
public:
  void init();
  void bind();

  void draw(
      const Mat3& PV,
      unsigned int font_texture,
      const Glyph2dInstance* data,
      size_t count);

private:
  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_PV;

  // Array/buffer objects
  unsigned int VAO;
  unsigned int quad_VBO;
  std::size_t quad_vertex_count = 0;
  unsigned int instance_VBO;
};

} // namespace dgui
