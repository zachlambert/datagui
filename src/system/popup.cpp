#include "datagui/system/popup.hpp"

namespace datagui {

void PopupSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& popup = element.popup();

  state.fixed_size = Vec2();
  state.dynamic_size = Vec2();
  state.floating = true;
  state.floating_type = FloatingTypeAbsolute(Vec2(popup.width, popup.height));
}

void PopupSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& popup = element.popup();

  Vec2 close_button_size =
      fm->text_size("x", theme->text_font, theme->text_size, LengthWrap()) +
      2.f * Vec2::uniform(theme->text_padding);

  popup.title_bar_text_width = std::max(
      state.float_box.size().x - close_button_size.x -
          2.f * theme->text_padding,
      0.f);

  {
    auto& box = popup.title_bar_box;
    box.lower = state.float_box.lower;
    box.upper.x = state.float_box.upper.x;
    box.upper.y = state.float_box.lower.y + close_button_size.y;
  }

  {
    auto& box = popup.close_button_box;
    box.lower.x = popup.title_bar_box.upper.x - close_button_size.x;
    box.lower.y = popup.title_bar_box.lower.y;
    box.upper = popup.title_bar_box.upper;
  }

  auto child = element.child();
  if (!child) {
    state.child_mask = state.box();
    return;
  }
  for (auto other = child.next(); other; other = other.next()) {
    other.state().hidden = true;
  }

  child.state().position = popup.title_bar_box.upper_left();
  child.state().size = state.float_box.upper - child.state().position;
  state.child_mask = child.state().box();
}

void PopupSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& popup = element.popup();

  const Color& header_color =
      popup.header_color ? *popup.header_color : theme->layout_color_bg;
  const Color& bg_color =
      popup.bg_color ? *popup.bg_color : theme->layout_color_bg;

  renderer.queue_box(state.float_box, bg_color);
  renderer.queue_box(popup.title_bar_box, header_color);

  renderer.queue_text(
      state.float_box.lower +
          Vec2::uniform(theme->input_border_width + theme->text_padding),
      popup.title,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthFixed(popup.title_bar_text_width));

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
