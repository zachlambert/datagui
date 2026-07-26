#pragma once

#include "datagui/render/program/shape_2d_program.hpp"
#include "datagui/render/program/glyph_2d_program.hpp"
#include "datagui/render/program/image_2d_program.hpp"

namespace dgui {

struct ProgramRegistry {
  Shape2dProgram shape_2d_program;
  Glyph2dProgram glyph_2d_program;
  Image2dProgram image_2d_program;

  void init();
};

} // namespace dgui
