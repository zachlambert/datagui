#include "datagui/system/button.hpp"

namespace datagui {

void ButtonSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& button = element.button();

  state.fixed_size =
      2.f * Vecf::Constant(theme->input_border_width + theme->text_padding);
  state.dynamic_size = Vecf::Zero();
  state.floating = false;

  Vecf text_size = fm->text_size(
      button.text,
      theme->text_font,
      theme->text_size,
      button.width);
  state.fixed_size.y += text_size.y;

  if (auto width = std::get_if<LengthFixed>(&button.width)) {
    state.fixed_size.x += width->value;
  } else {
    state.fixed_size.x += text_size.x;
    if (auto width = std::get_if<LengthDynamic>(&button.width)) {
      state.dynamic_size.x = width->weight;
    }
  }
}

void ButtonSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& button = element.button();

  Color bg_color;
  if (button.down) {
    bg_color = theme->input_color_bg_active;
  } else if (state.hovered) {
    bg_color = theme->input_color_bg_hover;
  } else {
    bg_color = theme->input_color_bg;
  }

  Color border_color;
  if (state.in_focus_tree) {
    border_color = theme->input_color_border_focus;
  } else {
    border_color = theme->input_color_border;
  }

  renderer.queue_box(
      state.box(),
      bg_color,
      theme->input_border_width,
      border_color,
      theme->input_radius);

  Vecf text_position =
      state.position +
      Vecf::Constant(theme->input_border_width + theme->text_padding);

  renderer.queue_text(
      text_position,
      button.text,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      button.width);
}

bool ButtonSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& button = element.button();

  if (event.button != MouseButton::Left) {
    return false;
  }
  switch (event.action) {
  case MouseAction::Press:
    button.down = true;
    break;
  case MouseAction::Release:
    button.down = false;
    if (state.box().contains(event.position)) {
      button.released = true;
      return true;
    }
    break;
  default:
    break;
  }
  return false;
}

bool ButtonSystem::key_event(ElementPtr element, const KeyEvent& event) {
  auto& button = element.button();
  if (event.key != Key::Enter) {
    return false;
  }
  switch (event.action) {
  case KeyAction::Press:
    button.down = true;
    break;
  case KeyAction::Release:
    button.down = false;
    button.released = true;
    return true;
  default:
    break;
  }
  return false;
}

} // namespace datagui
