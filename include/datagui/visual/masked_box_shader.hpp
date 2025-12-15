#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include <stack>
#include <vector>

namespace datagui {

class MaskedBoxShader {
public:
  void init();

  void queue_box(
      const Box2& mask,
      const Box2& box,
      const Color& color,
      float border_width = 0,
      Color border_color = Color(),
      float radius = 0);

  void draw(const Box2& viewport);
  void clear();

private:
  struct Element {
    Box2 box;
    Box2 mask;
    Color color;
    Color border_color;
    float border_width;
    float radius;
  };
  std::vector<Element> elements;
  std::stack<Box2> masks;

  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_viewport_lower;
  unsigned int uniform_viewport_upper;

  // Array/buffer objects
  unsigned int VAO;
  unsigned int static_VBO;
  unsigned int instance_VBO;
  std::size_t static_vertex_count = 0;
};

} // namespace datagui
