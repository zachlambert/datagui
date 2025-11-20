#pragma once

#include "datagui/geometry.hpp"
#include "datagui/layout.hpp"

namespace datagui {

struct Element {
  Vecf fixed_size;
  Vecf dynamic_size;
  bool hidden = false;
  bool zero_size() const {
    return fixed_size == Vecf::Zero() && dynamic_size == Vecf::Zero();
  }

  // Layout output
  Vecf position;
  Vecf size;
  Boxf box() const {
    return Boxf(position, position + size);
  }

  bool floating = false;
  FloatingType floating_type = FloatingTypeRelative(Vecf::Zero(), Vecf::Zero());
  Boxf float_box;
  int float_priority = 0;

  // Event handling
  bool in_focus_tree = false;
  bool focused = false;
  bool hovered = false;
};

} // namespace datagui
