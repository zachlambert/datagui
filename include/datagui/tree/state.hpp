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

  // Focus
  bool in_focus_tree = false;
  bool focused = false;
};

} // namespace datagui
