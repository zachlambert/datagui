#include "datagui/system/floating.hpp"

namespace datagui {

void FloatingSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& floating = element.floating();

  state.fixed_size = Vecf::Zero();
  state.dynamic_size = Vecf::Zero();
  state.floating = true;
  state.floating_type =
      FloatingTypeAbsolute(Vecf(floating.width, floating.height));
}

void FloatingSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& floating = element.floating();

  {
    auto& box = floating.title_bar_box;
    box.lower = state.float_box.lower;
    box.upper.x = state.float_box.upper.x;
    box.upper.y = state.float_box.lower.y +
                  fm->text_height(theme->text_font, theme->text_size) +
                  2.f * (theme->input_border_width + theme->text_padding);
  }

  floating.title_bar_text_width =
      state.float_box.size().x -
      2.f * (theme->input_border_width + theme->text_padding);

  {
    auto& box = floating.close_button_box;
    Vecf text_size = fm->text_size(
        "close",
        theme->text_font,
        theme->text_size,
        LengthWrap());

    Vecf button_size = text_size + 2.f * Vecf::Constant(theme->text_padding);
    floating.title_bar_text_width -= (button_size.x + theme->text_padding);
    floating.title_bar_text_width =
        std::max(0.f, floating.title_bar_text_width);

    box.upper = floating.title_bar_box.upper -
                Vecf::Constant(theme->input_border_width);
    box.lower = box.upper - button_size;
  }

  auto child = element.child();
  if (!child) {
    return;
  }
  for (auto other = child.next(); other; other = other.next()) {
    other.state().hidden = true;
  }

  child.state().position = floating.title_bar_box.top_left();
  child.state().size = state.float_box.upper - child.state().position;
}

void FloatingSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& floating = element.floating();

  const Color& bg_color =
      floating.bg_color ? *floating.bg_color : theme->layout_color_bg;

  renderer.queue_box(
      state.float_box,
      bg_color,
      theme->popup_border_width,
      Color::Black(),
      0);

  renderer.queue_box(
      floating.title_bar_box,
      theme->layout_color_bg,
      theme->input_border_width,
      theme->input_color_border,
      0);

  renderer.queue_text(
      state.float_box.lower +
          Vecf::Constant(theme->input_border_width + theme->text_padding),
      floating.title,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthFixed(floating.title_bar_text_width));

  renderer.queue_box(
      floating.close_button_box,
      theme->input_color_bg,
      0,
      Color::Black(),
      0);

  renderer.queue_text(
      floating.close_button_box.lower + Vecf::Constant(theme->text_padding),
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
  floating.open = false;
  floating.open_changed = true;
  return true;
}

} // namespace datagui
