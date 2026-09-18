#pragma once

#include "datagui/geometry/mat.hpp"
#include "datagui/render/state/shape_3d_instance.hpp"
#include <cstddef>
#include <vector>

namespace dgui {

class Shape3dProgram {
public:
  void init();
  void bind();

  void draw(
      const Mat4& view,
      const Mat4& projection,
      Shape3dType type,
      const Shape3dInstance* data,
      size_t count);

private:
  struct Shape {
    std::size_t indices_begin;
    std::size_t indices_end;
  };
  std::vector<Shape> shapes;

  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_P;
  unsigned int uniform_V;

  // Array/buffer objects
  unsigned int VAO;
  unsigned int static_VBO;
  unsigned int static_EBO;
  unsigned int instance_VBO;
};

} // namespace dgui
