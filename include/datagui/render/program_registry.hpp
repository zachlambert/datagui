#pragma once

#include "datagui/render/program/shape_2d_program.hpp"
#include "datagui/render/program/glyph_2d_program.hpp"
#include "datagui/render/program/image_2d_program.hpp"
#include "datagui/render/program/mesh_program.hpp"
#include "datagui/render/program/point_cloud_program.hpp"
#include "datagui/render/program/shape_3d_program.hpp"

namespace dgui {

struct ProgramRegistry {
  Shape2dProgram shape_2d_program;
  Glyph2dProgram glyph_2d_program;
  Image2dProgram image_2d_program;

  MeshProgram mesh_program;
  PointCloudProgram point_cloud_program;
  Shape3dProgram shape_3d_program;

  void init();
};

} // namespace dgui
