#pragma once

#include "datagui/render/state/shape_2d_instance.hpp"

namespace dgui {

class Shape2dProgram {
public:
  void init();
  void bind();

  void draw(const Shape2dInstance* data, size_t count, const Mat3& PV);

private:
  unsigned int program_id;
  unsigned int uniform_PV;
  unsigned int VAO;
  unsigned int static_VBO;
  unsigned int instance_VBO;
  std::size_t static_vertex_count = 0;
};

} // namespace dgui
