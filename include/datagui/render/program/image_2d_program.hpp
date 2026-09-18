#pragma once

#include "datagui/geometry/mat.hpp"

namespace dgui {

class Image2dProgram {
public:
  void init();
  void bind();
  void draw(
      const Mat3& PV,
      unsigned int texture,
      const Mat3& transform);

private:
  unsigned int program_id;
  unsigned int uniform_PVM;
  unsigned int VAO;
  unsigned int quad_VBO;
  unsigned int quad_vertex_count;
};

} // namespace dgui
