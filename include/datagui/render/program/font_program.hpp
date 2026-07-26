#pragma once

#include "datagui/geometry.hpp"

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
  unsigned int program_id;
  unsigned int uniform_PV;
  unsigned int uniform_text_color;
  unsigned int VAO, VBO;
};

} // namespace dgui
