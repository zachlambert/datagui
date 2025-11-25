#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/layout.hpp"
#include <vector>

namespace datagui {

class BoxShader {
public:
  void init();

  void queue_box(
      const Box2& box,
      const Color& bg_color,
      float border_width,
      Color border_color,
      int z_index,
      const Box2& mask);

  void render(const Vec2& viewport_size, int end_z_index);

private:
  struct {
    // Shader
    unsigned int program_id;
    // Uniforms
    unsigned int uniform_viewport_size;
    unsigned int uniform_max_depth;
    unsigned int uniform_end_z_index;
    // Array/buffer objects
    unsigned int VAO, static_VBO, instance_VBO;
  } gl_data;

  struct Element {
    Vec2 offset;
    Vec2 size;
    Color bg_color;
    Color border_color;
    float border_width;
    Box2 mask;
    int z_index;
  };
  std::vector<Element> elements;
};

} // namespace datagui
