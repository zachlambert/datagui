#include "datagui/render/state/shape_2d_instance.hpp"
#include "datagui/geometry/rot.hpp"

namespace dgui {

Shape2dInstance Shape2dInstance::box(
    const Box2& box,
    const Color& color,
    float border_width,
    Color border_color,
    float radius) {
  Shape2dInstance instance;
  instance.M = Mat3::transform(box.center(), Rot2(0.f), box.size());
  instance.color = color;
  instance.border_color = border_color;
  instance.border_width = Vec2::uniform(border_width) / box.size();
  instance.radius = Vec2::uniform(radius) / box.size();
  return instance;
}

Shape2dInstance Shape2dInstance::rect(
    const Vec2& position,
    float angle,
    const Vec2& size,
    const Color& color,
    float border_width,
    Color border_color) {
  Shape2dInstance instance;
  instance.M = Mat3::transform(position, Rot2(angle), size);
  instance.color = color;
  instance.border_color = border_color;
  instance.border_width = Vec2::uniform(border_width) / size;
  instance.radius = Vec2();
  return instance;
}

Shape2dInstance Shape2dInstance::circle(
    const Vec2& position,
    float radius,
    const Color& color,
    float border_width,
    Color border_color) {
  Shape2dInstance instance;
  instance.M = Mat3::transform(position, Rot2(0.f), Vec2::uniform(2 * radius));
  instance.color = color;
  instance.border_width = Vec2::uniform(border_width / (2 * radius));
  instance.border_color = border_color;
  instance.radius = Vec2::uniform(0.5);
  return instance;
}

Shape2dInstance Shape2dInstance::ellipse(
    const Vec2& position,
    float angle,
    const Vec2& radii,
    const Color& color,
    float border_width,
    Color border_color) {
  Shape2dInstance instance;
  instance.M = Mat3::transform(position, Rot2(angle), 2 * radii);
  instance.color = color;
  instance.border_width = Vec2::uniform(border_width) / (2 * radii);
  instance.border_color = border_color;
  instance.radius = Vec2::uniform(0.5);
  return instance;
}

Shape2dInstance Shape2dInstance::line(
    const Vec2& a,
    const Vec2& b,
    float width,
    const Color& color,
    bool rounded_ends) {
  Shape2dInstance instance;
  Vec2 position = (a + b) / 2;
  float angle = std::atan2(b.y - a.y, b.x - a.x);
  Vec2 size;
  Vec2 radius;
  if (rounded_ends) {
    size = Vec2((b - a).length() + width, width);
    radius = Vec2::uniform(width / 2);
  } else {
    size = Vec2((b - a).length(), width);
    radius = Vec2();
  }

  instance.M = Mat3::transform(position, Rot2(angle), size);
  instance.color = color;
  instance.border_width = Vec2();
  instance.radius = radius / size;
  return instance;
}

Shape2dInstance Shape2dInstance::capsule(
    const Vec2& start,
    const Vec2& end,
    float radius,
    const Color& color,
    float border_width,
    Color border_color) {
  Shape2dInstance instance;

  Vec2 position = (start + end) / 2;
  float angle = std::atan2(end.y - start.y, end.x - start.x);
  Vec2 size = Vec2((start - end).length() + 2 * radius, 2 * radius);

  instance.M = Mat3::transform(position, Rot2(angle), size);
  instance.color = color;
  instance.border_width = Vec2::uniform(border_width) / size;
  instance.radius = Vec2::uniform(radius) / size;
  instance.border_color = border_color;
  return instance;
}

} // namespace dgui
