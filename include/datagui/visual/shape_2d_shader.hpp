#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include <vector>

namespace datagui {

class Shape2dShader {
public:
  void init();

  void queue_masked_box(
      const Box2& mask,
      const Box2& box,
      const Color& color,
      float border_width = 0,
      Color border_color = Color(),
      float radius = 0);

  void queue_rect(
      const Vec2& position,
      float angle,
      const Vec2& size,
      const Color& color,
      float border_width = 0,
      Color border_color = Color());

  void queue_circle(
      const Vec2& position,
      float radius,
      const Color& color,
      float border_width = 0,
      Color border_color = Color());

  void queue_ellipse(
      const Vec2& position,
      float angle,
      const Vec2& radii,
      const Color& color,
      float border_width = 0,
      Color border_color = Color());

  void queue_line(
      const Vec2& a,
      const Vec2& b,
      float width,
      const Color& color,
      bool rounded_ends = true);

  void queue_capsule(
      const Vec2& start,
      const Vec2& end,
      float radius,
      const Color& color,
      float border_width = 0,
      Color border_color = Color());

  void draw(const Box2& viewport, const Camera2d& camera);
  void clear();

private:
  struct Element {
    Mat3 M;
    Color color;
    Color border_color;
    Vec2 border_width;
    Vec2 radius;
    Box2 mask = Box2(Vec2(-0.5, -0.5), Vec2(0.5, 0.5));
  };
  std::vector<Element> elements;

  // Shader
  unsigned int program_id;

  // Uniforms
  unsigned int uniform_PV;

  // Array/buffer objects
  unsigned int VAO;
  unsigned int static_VBO;
  unsigned int instance_VBO;
  std::size_t static_vertex_count = 0;
};

} // namespace datagui
