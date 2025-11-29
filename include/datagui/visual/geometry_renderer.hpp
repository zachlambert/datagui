#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/layout.hpp"
#include <vector>

namespace datagui {

class GeometryRenderer {
public:
  void init();

  void queue_box(
      const Box2& box,
      const Color& bg_color,
      BoxDims border_width,
      Color border_color,
      float radius,
      const Box2& mask);

  void render(const Vec2& viewport_size);

private:
  struct {
    // Shader
    unsigned int program_id;
    // Uniforms
    unsigned int uniform_viewport_size;
    unsigned int uniform_max_depth;
    // Array/buffer objects
    unsigned int VAO, static_VBO, instance_VBO;
  } gl_data;

  struct Element {
    Vec2 offset;
    Vec2 size;
    float radius;
    Color bg_color;
    Color border_color;
    float border_width;
    Box2 mask;
    float depth;
  };
  std::vector<Element> elements;
};

} // namespace datagui
