#pragma once

#include "datagui/geometry.hpp"

namespace datagui {

struct State {
  // Layout input
  Vecf fixed_size;
  Vecf dynamic_size;
  bool zero_size() const {
    return fixed_size == Vecf::Zero() && dynamic_size == Vecf::Zero();
  }

  // Layout output
  Vecf position;
  Vecf size;
  Boxf box() const {
    return Boxf(position, position + size);
  }

  Boxf layer_box;
  int layer = 0;
  bool floating = false;
  Boxf float_box;
  int float_priority = 0;

  // Event handling
  Boxf hitbox; // = box or float_box
  Boxf hitbox_bounds;
  bool in_focus_tree = false;
  bool focused = false;
  bool hovered = false;
};

} // namespace datagui
