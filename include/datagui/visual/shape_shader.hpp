#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include <vector>

namespace datagui {

class ShapeShader {
  struct Element {
    Vec2 position; // Centre
    Rot2 rotation;
    Vec2 size;
    Vec2 radius;
    Color color;
    Box2 mask;
  };

public:
  class Command {
  public:
    void queue_box(
        const Box2& box,
        const Color& color,
        float radius,
        float border_width,
        Color border_color,
        const Box2& mask);

    void queue_rect(
        const Vec2& position,
        float angle,
        const Vec2& size,
        const Color& color,
        float radius,
        float border_width,
        Color border_color,
        const Box2& mask);

    void queue_capsule(
        const Vec2& start,
        const Vec2& end,
        float radius,
        const Color& color,
        float border_width,
        Color border_color,
        const Box2& mask);

    void queue_circle(
        const Vec2& position,
        float radius,
        const Color& color,
        float border_width,
        Color border_color,
        const Box2& mask);

    void queue_ellipse(
        const Vec2& position,
        float angle,
        float x_radius,
        float y_radius,
        const Color& color,
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
  void draw(const Command& command, float y_dir, const Vec2& viewport_size);

private:
  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_y_dir;
  unsigned int uniform_viewport_size;

  // Array/buffer objects
  unsigned int VAO;
  unsigned int static_VBO;
  unsigned int instance_VBO;
};

} // namespace datagui
