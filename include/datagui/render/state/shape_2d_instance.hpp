#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry/box.hpp"
#include "datagui/geometry/mat.hpp"
#include "datagui/geometry/vec.hpp"

namespace dgui {

struct Shape2dInstance {
  Mat3 M;
  Color color;
  Color border_color;
  Vec2 border_width;
  Vec2 radius;

  static Shape2dInstance box(
      const Box2& box,
      const Color& color,
      float border_width,
      Color border_color,
      float radius);

  static Shape2dInstance rect(
      const Vec2& position,
      float angle,
      const Vec2& size,
      const Color& color,
      float border_width,
      Color border_color);

  static Shape2dInstance circle(
      const Vec2& position,
      float radius,
      const Color& color,
      float border_width,
      Color border_color);

  static Shape2dInstance ellipse(
      const Vec2& position,
      float angle,
      const Vec2& radii,
      const Color& color,
      float border_width,
      Color border_color);

  static Shape2dInstance line(
      const Vec2& a,
      const Vec2& b,
      float width,
      const Color& color,
      bool rounded_ends);

  static Shape2dInstance capsule(
      const Vec2& start,
      const Vec2& end,
      float radius,
      const Color& color,
      float border_width,
      Color border_color);
};

} // namespace dgui
