#pragma once

#include "datagui/color.hpp"
#include "datagui/font.hpp"
#include "datagui/layout.hpp"
#include <functional>

namespace datagui {

struct BoxStyle {
  BoxDims padding = BoxDims();
  BoxDims border_width = BoxDims();
  Color border_color = Color::Black();
  Color bg_color = Color::Clear();
  float radius = 0;
};

struct LinearLayoutStyle : public BoxStyle {
  Length length = literals::_wrap;
  Length width = literals::_wrap;
  float inner_padding;
};

struct TextStyle {
  Font font = Font::DejaVuSans;
  int font_size = 24;
  Color text_color = Color::Black();
  Length text_width = literals::_wrap;
};

struct SelectableTextStyle : public TextStyle {
  float cursor_width = 2;
  Color cursor_color = Color::Gray(0.5);
  Color highlight_color = Color::Gray(0.7);
  bool disabled = false;
};

template <typename Style>
using SetStyle = std::function<void(Style&)>;

} // namespace datagui
