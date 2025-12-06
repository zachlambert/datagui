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
  theme.layout_border_width = 4;
  theme.layout_border_color = Color::Gray(0.3);

  theme.scroll_bar_width = 6;
  theme.scroll_bar_bg = Color::Gray(0.8);
  theme.scroll_bar_fg = Color::Gray(0.3);

  theme.input_color_bg = Color::Gray(0.9);
  theme.input_color_bg_hover = Color::Gray(0.7);
  theme.input_color_bg_active = Color::Gray(0.5);
  theme.input_color_border = Color::Gray(0.2);
  theme.input_color_border_focus = Color::Gray(0.3);
  theme.input_border_width = 2;

  theme.slider_default_length = 100;
  theme.slider_height = 20;
  theme.slider_width = 8;
  theme.slider_color = Color::Gray(0.7);
  theme.slider_color_active = Color::Gray(0.3);
  theme.slider_bg_color = Color::Gray(0.9);
  theme.slider_border_width = 2;
  theme.slider_border_color = Color::Gray(0.2);

  theme.color_picker_icon_size = 30;
  theme.color_picker_padding = 10;
  theme.color_picker_hue_wheel_radius = 100;
  theme.color_picker_value_scale_width = 20;

  theme.header_color = Color::Gray(0.6);

  theme.split_divider_width = 8;
  theme.split_divider_color = Color::Gray(0.85);
  theme.split_divider_color_active = Color::Gray(0.5);

  return theme;
}

} // namespace datagui
