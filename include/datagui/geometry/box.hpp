#pragma once

#include "datagui/geometry/mat.hpp"
#include "datagui/geometry/vec.hpp"

namespace dgui {

struct Box2 {
  Vec2 lower;
  Vec2 upper;

  Box2() {}
  Box2(const Vec2& lower, const Vec2& upper) : lower(lower), upper(upper) {}

  static Box2 unit_box() {
    return Box2(Vec2(), Vec2::ones());
  }

  static Box2 from_size(const Vec2& pos, const Vec2& size) {
    return Box2(pos, pos + size);
  }

  bool contains(const Vec2& point) const {
    return point.x >= lower.x && point.x < upper.x && point.y >= lower.y &&
           point.y < upper.y;
  }

  Vec2 center() const {
    return Vec2((lower.x + upper.x) / 2, (lower.y + upper.y) / 2);
  }
  float center_x() const {
    return (upper.x + lower.x) / 2;
  }
  float center_y() const {
    return (upper.y + lower.y) / 2;
  }

  Vec2 size() const {
    return Vec2((upper.x - lower.x), (upper.y - lower.y));
  }
  float size_x() const {
    return upper.x - lower.x;
  }
  float size_y() const {
    return upper.y - lower.y;
  }

  float area() const {
    return (upper.x - lower.x) * (upper.y - lower.y);
  }

  bool empty() const {
    return lower.x >= upper.x || lower.y >= upper.y;
  }

  Vec2 lower_left() const {
    return lower;
  }
  Vec2 lower_right() const {
    return Vec2(upper.x, lower.y);
  }
  Vec2 upper_left() const {
    return Vec2(lower.x, upper.y);
  }
  Vec2 upper_right() const {
    return upper;
  }

  Vec2 lower_left(const Vec2& offset) const {
    return lower + offset;
  }
  Vec2 lower_right(const Vec2& offset) const {
    return Vec2(upper.x - offset.x, lower.y + offset.y);
  }
  Vec2 upper_left(const Vec2& offset) const {
    return Vec2(lower.x + offset.x, upper.y - offset.y);
  }
  Vec2 upper_right(const Vec2& offset) const {
    return upper - offset;
  }

  Vec2 lower_center() const {
    return Vec2((lower.x + upper.x) / 2, lower.y);
  }
  Vec2 upper_center() const {
    return Vec2((lower.x + upper.x) / 2, upper.y);
  }
  Vec2 center_left() const {
    return Vec2(lower.x, (lower.y + upper.y) / 2);
  }
  Vec2 center_right() const {
    return Vec2(upper.x, (lower.y + upper.y) / 2);
  }

  static Box2 from_lower_left(const Vec2& lower_left, const Vec2& size) {
    return Box2::from_size(lower_left, size);
  }
  static Box2 from_lower_right(const Vec2& lower_right, const Vec2& size) {
    return Box2::from_size(lower_right - Vec2(size.x, 0), size);
  }
  static Box2 from_upper_left(const Vec2& upper_left, const Vec2& size) {
    return Box2::from_size(upper_left - Vec2(0, size.y), size);
  }
  static Box2 from_upper_right(const Vec2& upper_right, const Vec2& size) {
    return Box2::from_size(upper_right - size, size);
  }

  void expand(float amount) {
    lower -= Vec2::uniform(amount);
    upper += Vec2::uniform(amount);
  }
  void expand(const Vec2& amount) {
    lower -= amount;
    upper += amount;
  }
  void shrink(float amount) {
    lower += Vec2::uniform(amount);
    upper -= Vec2::uniform(amount);
  }
  void shrink(const Vec2& amount) {
    lower += amount;
    upper -= amount;
  }

  Box2 from_expand(float amount) const {
    Box2 result = *this;
    result.expand(amount);
    return result;
  }
  Box2 from_expand(const Vec2& amount) const {
    Box2 result = *this;
    result.expand(amount);
    return result;
  }
  Box2 from_shrink(float amount) const {
    Box2 result = *this;
    result.shrink(amount);
    return result;
  }
  Box2 from_shrink(const Vec2& amount) const {
    Box2 result = *this;
    result.shrink(amount);
    return result;
  }

  // [lower, upper] -> [0, 1]
  Vec2 to_coords(const Vec2& position) const {
    return Vec2(
        (position.x - lower.x) / (upper.x - lower.x),
        (position.y - lower.y) / (upper.y - lower.y));
  }
  // [0, 1] -> [lower, upper]
  Vec2 from_coords(const Vec2& coords) const {
    return Vec2(
        lower.x + coords.x * (upper.x - lower.x),
        lower.y + coords.y * (upper.y - lower.y));
  }
  Box2 subview(const Box2& box_coords) const {
    return Box2(from_coords(box_coords.lower), from_coords(box_coords.upper));
  }

  float ratio_yx() const {
    return (upper.y - lower.y) / (upper.x - lower.x);
  }

  // Maps [-1, +1] within the box to the [lower, upper]
  Mat3 viewport_transform() const {
    return Mat3{
        {(upper.x - lower.x) / 2, 0.f, (lower.x + upper.x) / 2},
        {0.f, (upper.y - lower.y) / 2, (lower.y + upper.y) / 2},
        {0.f, 0.f, 1.f}};
  }
};

inline bool intersects(const Box2& a, const Box2& b) {
  return b.upper >= a.lower && b.lower <= a.upper;
}

inline Box2 intersection(const Box2& a, const Box2& b) {
  return Box2(maximum(a.lower, b.lower), minimum(a.upper, b.upper));
}

inline Box2 bounding(const Box2& a, const Box2& b) {
  return Box2(minimum(a.lower, b.lower), maximum(a.upper, b.upper));
}

inline Vec2 remap(const Vec2& point, const Box2& from, const Box2& to) {
  return to.from_coords(from.to_coords(point));
}

inline Vec2 remap_flip_y(const Vec2& point, const Box2& from, const Box2& to) {
  Vec2 coords = from.to_coords(point);
  coords.y = 1 - coords.y;
  return to.from_coords(coords);
}

// Returns true if outer_box containers inner_box
inline bool contains(const Box2& outer_box, const Box2& inner_box) {
  return inner_box.lower >= outer_box.lower &&
         inner_box.upper <= outer_box.upper;
}

#ifdef DGUI_DEBUG
std::ostream& operator<<(std::ostream& os, const Box2& box);
#endif

} // namespace dgui
