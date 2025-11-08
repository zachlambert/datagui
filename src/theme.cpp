#include "datagui/theme.hpp"

namespace datagui {

Theme theme_default() {
  Theme theme;

  theme.text_font = Font::DejaVuSans;
  theme.text_size = 14;
  theme.text_padding = 4;
  theme.text_color = Color::Black();
  theme.text_highlight_color = Color::Gray(0.8);
  theme.text_cursor_width = 2;
  theme.text_cursor_color = Color::Gray(0.2);

  theme.layout_color_bg = Color::Gray(0.7);
  theme.layout_outer_padding = 10;
  theme.layout_inner_padding = 4;

  theme.scroll_bar_width = 6;
  theme.scroll_bar_bg = Color::Gray(0.8);
  theme.scroll_bar_fg = Color::Gray(0.3);

  theme.input_color_bg = Color::Gray(0.9);
  theme.input_color_bg_hover = Color::Gray(0.7);
  theme.input_color_bg_active = Color::Gray(0.5);
  theme.input_color_border = Color::Gray(0.2);
  theme.input_color_border_focus = Color::Gray(0.3);
  theme.input_border_width = 2;
  theme.input_radius = 0;

  theme.popup_border_width = 4;
  theme.popup_margin = 30;

  return theme;
}

} // namespace datagui
