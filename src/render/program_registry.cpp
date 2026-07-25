#include "datagui/render/program_registry.hpp"

namespace dgui {

void ProgramRegistry::init() {
  shape_2d_program.init();
  glyph_2d_program.init();
  image_2d_program.init();
}

} // namespace dgui
