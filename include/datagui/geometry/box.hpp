#pragma once

#include "datagui/geometry/mat.hpp"
#include "datagui/geometry/vec.hpp"

namespace dgui {

struct Box2 {
  Vec2 lower;
  Vec2 upper;

  Box2() {}
  Box2(const Vec2& lower, const Vec2& upper) : lower(lower), upper(upper) {}
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

  // To local box coords [0, 1]
  Vec2 to_coords(const Vec2& position) {
    return Vec2(
        (position.x - lower.x) / (upper.x - lower.x),
        (position.y - lower.y) / (upper.y - lower.y));
  }
  // From local box coords [0, 1]
  Vec2 from_local(const Vec2& coords) {
    return Vec2(
        lower.x + coords.x * (upper.x - lower.x),
        lower.y + coords.y * (upper.y - lower.y));
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

// Returns true if outer_box containers inner_box
inline bool contains(const Box2& outer_box, const Box2& inner_box) {
  return inner_box.lower >= outer_box.lower &&
         inner_box.upper <= outer_box.upper;
}

#ifdef DGUI_DEBUG
std::ostream& operator<<(std::ostream& os, const Box2& box);
#endif

} // namespace dgui
