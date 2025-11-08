#pragma once

#include "datagui/color.hpp"
#include "datagui/font.hpp"

namespace datagui {

struct Theme {
  Font text_font = Font::DejaVuSans;
  int text_size = 0;
  float text_padding = 0;
  Color text_color = Color::Black();
  Color text_highlight_color = Color::Black();
  float text_cursor_width = 0;
  Color text_cursor_color = Color::Black();

  Color layout_color_bg = Color::White();
  float layout_outer_padding = 0;
  float layout_inner_padding = 0;

  float scroll_bar_width = 0;
  Color scroll_bar_bg = Color::Black();
  Color scroll_bar_fg = Color::Black();

  Color input_color_bg = Color::Black();
  Color input_color_bg_hover = Color::Black();
  Color input_color_bg_active = Color::Black();
  Color input_color_border = Color::Black();
  Color input_color_border_focus = Color::Black();
  float input_border_width = 0;
  float input_radius = 0;

  float popup_border_width = 0;
};

Theme theme_default();

#if 0
struct TextStyle {
  Font font = Font::DejaVuSans;
  int font_size = 0;
  Color text_color = Color::Black();
  Color highlight_color = Color::Black();
  float cursor_width = 0;
  Color cursor_color = Color::Black();
};

struct InputStyle {
  float active_color_factor = 0.6;
  float hover_color_factor = 0.8;
  float focus_color_factor = 0.8;
};
#endif

} // namespace datagui
