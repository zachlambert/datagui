#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/layout.hpp"
#include "datagui/style.hpp"
#include <vector>

namespace datagui {

class GeometryRenderer {
public:
  GeometryRenderer();
  void init();

  void queue_box(
      const Boxf& box,
      const Color& bg_color,
      BoxDims border_width,
      Color border_color,
      float radius);

  void queue_box(const Boxf& box, const BoxStyle& style) {
    queue_box(
        box,
        style.bg_color,
        style.border_width,
        style.border_color,
        style.radius);
  }

  void render(const Vecf& viewport_size);

private:
  struct {
    // Shader
    unsigned int program_id;
    // Uniforms
    unsigned int uniform_viewport_size;
    // Array/buffer objects
    unsigned int VAO, static_VBO, instance_VBO;
  } gl_data;

  struct Element {
    Vecf offset;
    Vecf size;
    float radius;
    Color bg_color;
    Color border_color;
    BoxDims border_width;
  };
  std::vector<Element> elements;
};

} // namespace datagui
