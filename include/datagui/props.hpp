#pragma once

#include "datagui/types/prop_stack.hpp"

namespace datagui {

enum class Prop {
  // Boxes
  Padding,
  BorderWidth,
  BorderColor,
  BgColor,
  Radius,
  Width,
  Height,

  // Layout
  LayoutInnerPadding,
  LayoutDirection,
  LayoutAlignment,
};

} // namespace datagui
