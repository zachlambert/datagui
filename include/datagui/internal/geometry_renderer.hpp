#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include <vector>

namespace datagui {

class GeometryRenderer {
public:
  GeometryRenderer();
  void init();

  void queue_box(
      const Boxf& box,
      const Color& color,
      float border_width,
      Color border_color,
      float radius = 0.f);

  void render(const Vecf& viewport_size);

private:
  struct BoxCommand {
    Boxi box;
    Color color;
  };

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
    float border_width;
    Color bg_color;
    Color border_color;
  };
  std::vector<Element> elements;
};

} // namespace datagui
