#pragma once

#include "datagui/color.hpp"
#include "datagui/layout.hpp"

namespace datagui {

struct LayoutStyle {
  BoxDims outer_padding = BoxDims();
  BoxDims border_width = BoxDims();
  Color border_color = Color::Black();
  Color bg_color = Color::Clear();
};

struct LinearLayoutStyle : public LayoutStyle {
  Length length = literals::_wrap;
  Length width = literals::_wrap;
  float inner_padding;
};

} // namespace datagui
