#pragma once

#include "datagui/render/shape_2d_program.hpp"

namespace dgui {

struct ProgramRegistry {
  Shape2dProgram shape_2d_program;

  void init();
};

} // namespace dgui
