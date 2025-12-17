#pragma once

#include "datagui/geometry/vec.hpp"

namespace datagui {

struct Box2 {
  Vec2 lower;
  Vec2 upper;

  Box2() {}
  Box2(const Vec2& lower, const Vec2& upper) : lower(lower), upper(upper) {}

  bool contains(const Vec2& point) const {
    return point.x >= lower.x && point.x < upper.x && point.y >= lower.y &&
           point.y < upper.y;
  }

  Vec2 center() const {
    return Vec2((lower.x + upper.x) / 2, (lower.y + upper.y) / 2);
  }

  Vec2 size() const {
    return Vec2((upper.x - lower.x), (upper.y - lower.y));
  }

  float area() const {
    return (upper.x - lower.x) * (upper.y - lower.y);
  }

  bool empty() const {
    return lower == upper;
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

#ifdef DATAGUI_DEBUG
std::ostream& operator<<(std::ostream& os, const Box2& box);
#endif

} // namespace datagui
