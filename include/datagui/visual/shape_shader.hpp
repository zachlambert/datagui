#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include <vector>

namespace datagui {

class ShapeShader {
  struct Element {
    Vec2 offset;
    Vec2 size;
    Color bg_color;
    Color border_color;
    float border_width;
    Box2 mask;
  };

public:
  class Command {
  public:
    void queue_box(
        const Box2& box,
        const Color& bg_color,
        float border_width,
        Color border_color,
        const Box2& mask);

    void clear() {
      elements.clear();
    }

  private:
    std::vector<Element> elements;
    friend class ShapeShader;
  };

  void init();
  void draw(const Command& command, const Vec2& viewport_size);

private:
  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_viewport_size;

  // Array/buffer objects
  unsigned int VAO;
  unsigned int static_VBO;
  unsigned int instance_VBO;
};

} // namespace datagui
