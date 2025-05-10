#include "datagui/theme.hpp"

namespace datagui {

Style default_theme() {
  Style style;

  // TODO
#if 0
  style.padding(5);
  style.border_color(Color::Black());
  style.bg_color(Color::White());
  style.radius(0);

  style.font(Font::DejaVuSans);
  style.font_size(24);
  style.text_color(Color::Black());
  style.highlight_color(Color::Gray(0.7));
  style.cursor_width(2);
  style.cursor_color(Color::Gray(0.4));

  style.layout_inner_padding(5);
  style.focus_color(Color(0, 0.4, 0.8));
  style.active_color(Color(0, 0.6, 0.9));
  style.hover_color(Color(0, 0.8, 1.0));

  style.checkbox_size(24);
  style.checkbox_bg_color(Color::White());
  style.checkbox_border_color(Color::Black());
  style.checkbox_border_width(2);
  style.checkbox_icon_color(Color(0, 0.6, 0.9));
  style.checkbox_inner_padding(2);

  style.title_bar_bg_color(Color(0, 0.3, 0.5));
  style.title_bar_border_color(Color::Black());
  style.title_bar_border_width(0);
  style.title_bar_padding(5);

  style.close_button_color(Color::Gray(0.5));
  style.close_button_padding(5);
#endif

  return style;
}

} // namespace datagui
