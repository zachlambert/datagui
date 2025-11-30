#include "datagui/system/floating.hpp"

namespace datagui {

void FloatingSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& floating = element.floating();

  state.fixed_size = Vec2();
  state.dynamic_size = Vec2();
  state.floating = true;
  state.floating_type =
      FloatingTypeAbsolute(Vec2(floating.width, floating.height));
}

void FloatingSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& floating = element.floating();

  Vec2 close_button_size =
      fm->text_size("close", theme->text_font, theme->text_size, LengthWrap()) +
      2.f * Vec2::uniform(theme->text_padding);

  floating.title_bar_text_width = std::max(
      state.float_box.size().x - close_button_size.x -
          2.f * theme->text_padding,
      0.f);

  {
    auto& box = floating.title_bar_box;
    box.lower = state.float_box.lower;
    box.upper.x = state.float_box.upper.x;
    box.upper.y = state.float_box.lower.y + close_button_size.y;
  }

  {
    auto& box = floating.close_button_box;
    box.lower.x = floating.title_bar_box.upper.x - close_button_size.x;
    box.lower.y = floating.title_bar_box.lower.y;
    box.upper = floating.title_bar_box.upper;
  }

  auto child = element.child();
  if (!child) {
    state.child_mask = state.box();
    return;
  }
  for (auto other = child.next(); other; other = other.next()) {
    other.state().hidden = true;
  }

  child.state().position = floating.title_bar_box.upper_left();
  child.state().size = state.float_box.upper - child.state().position;
  state.child_mask = child.state().box();
}

void FloatingSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& floating = element.floating();

  const Color& header_color =
      floating.header_color ? *floating.header_color : theme->layout_color_bg;
  const Color& bg_color =
      floating.bg_color ? *floating.bg_color : theme->layout_color_bg;

  renderer.queue_box(state.float_box, bg_color);
  renderer.queue_box(floating.title_bar_box, header_color);

  renderer.queue_text(
      state.float_box.lower +
          Vec2::uniform(theme->input_border_width + theme->text_padding),
      floating.title,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthFixed(floating.title_bar_text_width));

  renderer.queue_box(floating.close_button_box, theme->input_color_bg);

  renderer.queue_text(
      floating.close_button_box.lower + Vec2::uniform(theme->text_padding),
      "close",
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthWrap());
}

bool FloatingSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  auto& floating = element.floating();

  if (event.action != MouseAction::Release) {
    return false;
  }

  if (!floating.close_button_box.contains(event.position)) {
    return false;
  }
  floating.closed_callback();
  return false;
}

} // namespace datagui
