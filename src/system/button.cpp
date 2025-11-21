#include "datagui/element/button.hpp"

namespace datagui {

void ButtonSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  const auto& props = *e.props.cast<ButtonProps>();

  e.fixed_size =
      2.f * Vecf::Constant(theme->input_border_width + theme->text_padding);
  e.dynamic_size = Vecf::Zero();
  e.floating = false;

  Vecf text_size = fm->text_size(
      props.text,
      theme->text_font,
      theme->text_size,
      props.width);
  e.fixed_size.y += text_size.y;

  if (auto width = std::get_if<LengthFixed>(&props.width)) {
    e.fixed_size.x += width->value;
  } else {
    e.fixed_size.x += text_size.x;
    if (auto width = std::get_if<LengthDynamic>(&props.width)) {
      e.dynamic_size.x = width->weight;
    }
  }
}

void ButtonSystem::render(const Element& e, Renderer& renderer) {
  const auto& props = *e.props.cast<ButtonProps>();

  Color bg_color;
  if (props.down) {
    bg_color = theme->input_color_bg_active;
  } else if (e.hovered) {
    bg_color = theme->input_color_bg_hover;
  } else {
    bg_color = theme->input_color_bg;
  }

  Color border_color;
  if (e.in_focus_tree) {
    border_color = theme->input_color_border_focus;
  } else {
    border_color = theme->input_color_border;
  }

  renderer.queue_box(
      e.box(),
      bg_color,
      theme->input_border_width,
      border_color,
      theme->input_radius);

  Vecf text_position =
      e.position +
      Vecf::Constant(theme->input_border_width + theme->text_padding);

  renderer.queue_text(
      text_position,
      props.text,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      props.width);
}

bool ButtonSystem::mouse_event(Element& e, const MouseEvent& event) {
  auto& props = *e.props.cast<ButtonProps>();

  if (event.button != MouseButton::Left) {
    return false;
  }
  switch (event.action) {
  case MouseAction::Press:
    props.down = true;
    break;
  case MouseAction::Release:
    props.down = false;
    if (e.box().contains(event.position)) {
      props.released = true;
      return true;
    }
    break;
  default:
    break;
  }
  return false;
}

bool ButtonSystem::key_event(Element& e, const KeyEvent& event) {
  auto& props = *e.props.cast<ButtonProps>();
  if (event.key != Key::Enter) {
    return false;
  }
  switch (event.action) {
  case KeyAction::Press:
    props.down = true;
    break;
  case KeyAction::Release:
    props.down = false;
    props.released = true;
    return true;
  default:
    break;
  }
  return false;
}

} // namespace datagui
