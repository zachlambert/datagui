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
      const Boxf& box,
      const Color& bg_color,
      BoxDims border_width,
      Color border_color,
      float radius,
      const Boxf& mask);

  void render(const Vecf& viewport_size);

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
    Vecf offset;
    Vecf size;
    float radius;
    Color bg_color;
    Color border_color;
    float border_width;
    Boxf mask;
    float depth;
  };
  std::vector<Element> elements;
};

} // namespace datagui
