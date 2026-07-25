#pragma once

#include "datagui/render/shape_2d_program.hpp"
#include "datagui/render/glyph_2d_program.hpp"
#include "datagui/render/image_2d_program.hpp"

namespace dgui {

struct ProgramRegistry {
  Shape2dProgram shape_2d_program;
  Glyph2dProgram glyph_2d_program;
  Image2dProgram image_2d_program;

  void init();
};

} // namespace dgui
