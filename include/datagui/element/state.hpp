#pragma once

#include "datagui/geometry.hpp"
#include "datagui/layout.hpp"

namespace datagui {

struct State {
  Vec2 fixed_size;
  Vec2 dynamic_size;
  bool hidden = false;
  bool zero_size() const {
    return fixed_size == Vec2() && dynamic_size == Vec2();
  }

  // Layout output
  Vec2 position;
  Vec2 size;
  Box2 box() const {
    return Box2(position, position + size);
  }

  bool floating = false;
  FloatingType floating_type = FloatingTypeRelative(Vec2(), Vec2());
  Box2 float_box;
  int float_priority = 0;

  Box2 child_mask;

  // Event handling
  bool in_focus_tree = false;
  bool focused = false;
  bool hovered = false;
};

} // namespace datagui
