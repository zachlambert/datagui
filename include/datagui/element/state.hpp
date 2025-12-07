#pragma once

#include "datagui/geometry.hpp"
#include "datagui/layout.hpp"

namespace datagui {

struct State {
  // Layout input
  // Define for self set_independent_state(...)

  Vec2 fixed_size;
  Vec2 dynamic_size;

  // If true, then the element will have an additional floating component
  bool floating = false;
  // If true, then the element has no effect on it's parent layout
  bool float_only = false;
  FloatingType floating_type = FloatingTypeRelative(Vec2(), Vec2());

  // Layout output
  // Define in set_dependent_state(...)

  // Set on parent
  Box2 child_mask;

  Vec2 position;
  Vec2 size;
  Box2 box() const {
    return Box2(position, position + size);
  }

  bool hidden = false;

  // Other state

  Box2 float_box;
  int float_priority = 0;

  // Event handling
  bool in_focus_tree = false;
  bool focused = false;
  bool hovered = false;
};

} // namespace datagui
