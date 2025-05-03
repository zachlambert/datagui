#pragma once

#include "datagui/geometry.hpp"

namespace datagui {

struct State {
  // Layout input
  Vecf fixed_size;
  Vecf dynamic_size;

  // Layout output
  Vecf position;
  Vecf size;
  Boxf box() const {
    return Boxf(position, position + size);
  }

  // Layer calculation
  int layer_offset = 0; // If -1, then this is a window
  int max_layer = 0;
  int layer = 0;
  int window_priority = 0; // Only used if a window

  // Event handling
  Boxf hitbox;
  Boxf hitbox_bounds;
  bool in_focus_tree = false;
  bool focused = false;
  bool hovered = false;
};

} // namespace datagui
