#pragma once

#include "datagui/geometry.hpp"

namespace datagui {

enum class ElementType { LinearLayout, Text, TextInput };

struct LayoutInput {
  Vecf fixed_size;
  Vecf dynamic_size;
};

struct LayoutOutput {
  Vecf position;
  Vecf size;
};

struct State {
  ElementType element_type;
  int element_index;

  LayoutInput layout_input;
  LayoutOutput layout_output;
};

} // namespace datagui
