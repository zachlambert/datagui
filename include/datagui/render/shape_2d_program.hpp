#pragma once

#include "datagui/render/shape_2d_instance.hpp"

namespace dgui {

class Shape2dProgram {
public:
  void init();
  void draw(const Shape2dInstance* data, size_t count, const Mat3& PV);

private:
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_PV;

  // Array/buffer objects
  unsigned int VAO;
  unsigned int static_VBO;
  unsigned int instance_VBO;
  std::size_t static_vertex_count = 0;
};

} // namespace dgui
