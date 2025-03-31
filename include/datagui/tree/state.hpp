#pragma once

#include "datagui/geometry.hpp"

namespace datagui {

enum class ElementType { Undefined, LinearLayout, Text, TextInput };

struct State {
  ElementType element_type = ElementType::Undefined;
  int element_index = -1;

  // Layout input
  Vecf fixed_size;
  Vecf dynamic_size;

  // Layout output
  Vecf position;
  Vecf size;

  // Focus
  bool in_focus_tree = false;
  bool focused = false;
};

} // namespace datagui
