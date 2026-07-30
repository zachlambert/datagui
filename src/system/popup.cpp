#include "datagui/system/popup.hpp"
#include "datagui/system_utils/layout.hpp"

namespace dgui {

void PopupSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& popup = element.popup();

  state.fixed_size = Vec2();
  state.dynamic_size = Vec2();
  state.floating = popup.open;
  state.float_only = true;

  if (!popup.open) {
    return;
  }

  const auto& font =
      font_registry->get_font(theme->text_font, theme->text_size);
  popup.header_height = font.text_height() + 2 * theme->text_padding;

  layout_set_input_state(element, theme, popup.layout, popup.layout_state);

  state.floating_type =
      FloatingTypeAbsolute(Vec2(popup.popup_size.x, popup.popup_size.y));
}

void PopupSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& popup = element.popup();

  if (!popup.open) {
    return;
  }

  popup.header_box.lower = state.float_box.lower;
  popup.header_box.upper.x = state.float_box.upper.x;
  popup.header_box.upper.y = state.float_box.lower.y + popup.header_height;

  const auto& font =
      font_registry->get_font(theme->text_font, theme->text_size);
  Vec2 x_size = font.text_size("x", LengthWrap()) +
                Vec2::uniform(2 * theme->text_padding);

  popup.close_button_box.lower.x = state.float_box.upper.x - x_size.x;
  popup.close_button_box.lower.y = state.float_box.lower.y;
  popup.close_button_box.upper = popup.close_button_box.lower + x_size;

  popup.header_text_width = std::max(
      state.float_box.size().x - x_size.x - 2.f * theme->text_padding,
      0.f);

  auto& content_box = popup.layout_state.content_box;
  content_box = state.float_box;
  content_box.lower.y += popup.header_height;

  layout_set_dependent_state(element, theme, popup.layout, popup.layout_state);
}

void PopupSystem::render(ConstElementPtr element, DrawList& dl) {
  const auto& state = element.state();
  const auto& popup = element.popup();

  if (!popup.open) {
    return;
  }

  const auto& font =
      font_registry->get_font(theme->text_font, theme->text_size);

  const Color& header_color =
      popup.header_color ? *popup.header_color : theme->layout_color_bg;
  const Color& bg_color =
      popup.bg_color ? *popup.bg_color : theme->layout_color_bg;

  dl.draw_box(state.float_box, bg_color);
  dl.draw_box(popup.header_box, header_color);

  dl.draw_text(
      font,
      state.float_box.lower +
          Vec2::uniform(theme->input_border_width + theme->text_padding),
      theme->text_color,
      LengthFixed(popup.header_text_width),
      popup.title);

  dl.draw_box(popup.close_button_box, theme->input_color_bg);

  dl.draw_text(
      font,
      popup.close_button_box.lower + Vec2::uniform(theme->text_padding),
      theme->text_color,
      LengthWrap(),
      "x");

  layout_render(popup.layout_state, theme, dl);
}

void PopupSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  auto& popup = element.popup();

  if (event.action == MouseAction::Release &&
      popup.close_button_box.contains(event.position)) {
    popup.close_button_released = true;
  }
}

bool PopupSystem::scroll_event(ElementPtr element, const ScrollEvent& event) {
  auto& popup = element.popup();
  return layout_scroll_event(popup.layout_state, event);
}

} // namespace dgui
