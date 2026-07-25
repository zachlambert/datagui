#pragma once

#include "datagui/geometry.hpp"
#include "datagui/render/image_2d_vertex.hpp"

namespace dgui {

class Image2dProgram {
public:
  void init();

  void bind();
  void unbind();

  void draw(
      unsigned int texture,
      const Image2dVertex* data,
      size_t count,
      const Mat3& PV);

private:
  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_PV;

  // Array/buffer objects
  unsigned int VAO, VBO;
};

} // namespace dgui
