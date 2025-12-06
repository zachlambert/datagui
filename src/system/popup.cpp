#include "datagui/system/popup.hpp"
#include "datagui/system_utils/layout.hpp"

namespace datagui {

void PopupSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& popup = element.popup();

  state.fixed_size = Vec2();
  state.dynamic_size = Vec2();
  state.floating = true;

  popup.header_height = fm->text_height(theme->text_font, theme->text_size) +
                        2 * theme->text_padding;

  layout_set_input_state(element, theme, popup.layout, popup.layout_state);

  state.floating_type =
      FloatingTypeAbsolute(Vec2(popup.popup_size.x, popup.popup_size.y));
}

void PopupSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& popup = element.popup();

  popup.header_box.lower = state.float_box.lower;
  popup.header_box.upper.x = state.float_box.upper.x;
  popup.header_box.upper.y = state.float_box.lower.y + popup.header_height;

  Vec2 x_size =
      fm->text_size("x", theme->text_font, theme->text_size, LengthWrap()) +
      Vec2::uniform(2 * theme->text_padding);

  popup.close_button_box.lower.x = state.float_box.upper.x - x_size.x;
  popup.close_button_box.lower.y = state.float_box.lower.y;
  popup.close_button_box.upper = popup.close_button_box.lower + x_size;

  popup.header_text_width = std::max(
      state.float_box.size().x - x_size.x - 2.f * theme->text_padding,
      0.f);

  popup.content_box = state.float_box;
  popup.content_box.lower.y += popup.header_height;
  state.child_mask = popup.content_box;

  layout_set_dependent_state(
      element,
      popup.content_box,
      theme,
      popup.layout,
      popup.layout_state);
}

void PopupSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& popup = element.popup();

  const Color& header_color =
      popup.header_color ? *popup.header_color : theme->layout_color_bg;
  const Color& bg_color =
      popup.bg_color ? *popup.bg_color : theme->layout_color_bg;

  renderer.queue_box(state.float_box, bg_color);
  renderer.queue_box(popup.header_box, header_color);

  renderer.queue_text(
      state.float_box.lower +
          Vec2::uniform(theme->input_border_width + theme->text_padding),
      popup.title,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthFixed(popup.header_text_width));

  renderer.queue_box(popup.close_button_box, theme->input_color_bg);

  renderer.queue_text(
      popup.close_button_box.lower + Vec2::uniform(theme->text_padding),
      "x",
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthWrap());
}

void PopupSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  auto& popup = element.popup();

  if (event.action != MouseAction::Release) {
    return;
  }

  if (!popup.close_button_box.contains(event.position)) {
    return;
  }
  popup.closed_callback();
}

} // namespace datagui
