#include "datagui/element/button.hpp"

namespace datagui {

void ButtonSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  const auto& props = *e.props.cast<ButtonProps>();

  e.fixed_size = props.border_width.size() + props.padding.size();
  e.dynamic_size = Vecf::Zero();
  e.floating = false;

  Vecf text_size = fm->text_size(props.text, props.text_style, props.width);
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
    bg_color = props.bg_color.multiply(props.input_style.active_color_factor);
  } else if (e.hovered) {
    bg_color = props.bg_color.multiply(props.input_style.hover_color_factor);
  } else {
    bg_color = props.bg_color;
  }

  Color border_color;
  if (e.in_focus_tree) {
    border_color =
        props.border_color.multiply(props.input_style.focus_color_factor);
  } else {
    border_color = props.border_color;
  }

  renderer.queue_box(
      e.box(),
      bg_color,
      props.border_width,
      border_color,
      props.radius);

  Vecf text_position =
      e.position + props.border_width.offset() + props.padding.offset();

  renderer.queue_text(text_position, props.text, props.text_style, props.width);
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
