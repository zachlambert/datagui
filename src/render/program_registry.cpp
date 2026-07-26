#include "datagui/render/program_registry.hpp"

namespace dgui {

void ProgramRegistry::init() {
  shape_2d_program.init();
  glyph_2d_program.init();
  image_2d_program.init();

  mesh_program.init();
  uv_mesh_program.init();
  point_cloud_program.init();
  shape_3d_program.init();
}

} // namespace dgui
