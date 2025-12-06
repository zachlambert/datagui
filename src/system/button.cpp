#include "datagui/system/button.hpp"

namespace datagui {

void ButtonSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& button = element.button();

  state.fixed_size =
      2.f * Vec2::uniform(theme->input_border_width + theme->text_padding);
  state.dynamic_size = Vec2();
  state.floating = false;

  Vec2 text_size = fm->text_size(
      button.text,
      theme->text_font,
      theme->text_size,
      LengthWrap());
  state.fixed_size += text_size;
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
      border_color);

  Vec2 text_position =
      state.position +
      Vec2::uniform(theme->input_border_width + theme->text_padding);

  Color text_color = button.text_color ? *button.text_color : theme->text_color;
  int text_size = button.text_size != 0 ? button.text_size : theme->text_size;

  renderer.queue_text(
      text_position,
      button.text,
      theme->text_font,
      text_size,
      text_color,
      LengthWrap());
}

void ButtonSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& button = element.button();

  if (event.button != MouseButton::Left) {
    return;
  }
  switch (event.action) {
  case MouseAction::Press:
    button.down = true;
  case MouseAction::Release:
    button.down = false;
    if (state.box().contains(event.position)) {
      if (button.callback) {
        button.callback();
      } else {
        element.set_dirty();
      }
    }
    break;
  default:
    break;
  }
}

void ButtonSystem::key_event(ElementPtr element, const KeyEvent& event) {
  auto& button = element.button();
  if (event.key != Key::Enter) {
    return;
  }

  switch (event.action) {
  case KeyAction::Press:
    button.down = true;
    break;
  case KeyAction::Release:
    button.down = false;
    if (button.callback) {
      button.callback();
    } else {
      element.set_dirty();
    }
    break;
  default:
    break;
  }
}

} // namespace datagui
