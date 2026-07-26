#pragma once

#include "datagui/geometry.hpp"

namespace dgui {

class Image2dProgram {
public:
  void init();
  void bind();
  void draw(unsigned int texture, const Mat3& M, const Mat3& PV);

private:
  unsigned int program_id;
  unsigned int uniform_PVM;
  unsigned int VAO;
  unsigned int static_VBO;
  unsigned int static_vertex_count;
};

} // namespace dgui
