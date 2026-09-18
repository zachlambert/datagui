#pragma once

#include "datagui/render/state/shape_2d_instance.hpp"

namespace dgui {

class Shape2dProgram {
public:
  void init();
  void bind();

  void draw(
      const Mat3& PV,
      const Shape2dInstance* data,
      size_t count);

private:
  unsigned int program_id;
  unsigned int uniform_PV;
  unsigned int VAO;
  unsigned int quad_VBO;
  std::size_t quad_vertex_count = 0;
  unsigned int instance_VBO;
};

} // namespace dgui
