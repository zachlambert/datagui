#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/layout.hpp"
#include "datagui/style.hpp"
#include <stack>
#include <vector>

namespace datagui {

class GeometryRenderer {
public:
  GeometryRenderer();
  void init();

  void queue_box(
      const Boxf& box,
      float z_pos,
      const Color& bg_color,
      BoxDims border_width,
      Color border_color,
      float radius);

  void queue_box(const Boxf& box, float z_pos, const BoxStyle& style) {
    queue_box(
        box,
        z_pos,
        style.bg_color,
        style.border_width,
        style.border_color,
        style.radius);
  }

  void render(const Vecf& viewport_size);

  void push_mask(const Boxf& mask) {
    if (masks.empty()) {
      masks.push(mask);
    } else {
      masks.push(intersection(masks.top(), mask));
    }
  }
  void pop_mask() {
    masks.pop();
  }

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
    float z_pos;
    float radius;
    Color bg_color;
    Color border_color;
    BoxDims border_width;
    Boxf mask;
  };
  std::vector<Element> elements;

  std::stack<Boxf> masks;
};

} // namespace datagui
