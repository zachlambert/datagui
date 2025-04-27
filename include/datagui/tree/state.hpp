#pragma once

#include "datagui/geometry.hpp"

namespace datagui {

struct State {
  // System id
  int system_id;

  // Layout input
  Vecf fixed_size;
  Vecf dynamic_size;
  Boxf hitbox_offset;

  // Layout output
  Vecf position;
  Vecf size;
  Boxf box() const {
    return Boxf(position, position + size);
  }
  Boxf hitbox;
  Boxf bounding_box; // Bounds hitbox + child hitboxes
  Rangef z_range;

  // Event-handling
  bool in_focus_tree = false;
  bool focused = false;
  bool hovered = false;
};

} // namespace datagui
