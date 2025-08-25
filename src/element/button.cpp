#include "datagui/element/button.hpp"

namespace datagui {

bool ButtonSystem::visit(Element element, const std::string& text) {
  auto& data = element.data<ButtonData>();
  data.text = text;
  data.style.apply(res.style_manager);

  if (data.released) {
    data.released = false;
    return true;
  }
  return false;
}

void ButtonSystem::set_input_state(Element element) const {
  const auto& data = element.data<ButtonData>();
  const auto& style = data.style;

  element->fixed_size = style.border_width.size() + style.padding.size();
  element->dynamic_size = Vecf::Zero();
  element->floating = false;

  Vecf text_size =
      res.font_manager.text_size(data.text, style.text, style.width);
  element->fixed_size.y += text_size.y;

  if (auto width = std::get_if<LengthFixed>(&style.width)) {
    element->fixed_size.x += width->value;
  } else {
    element->fixed_size.x += text_size.x;
    if (auto width = std::get_if<LengthDynamic>(&style.width)) {
      element->dynamic_size.x = width->weight;
    }
  }
}

void ButtonSystem::render(ConstElement element) const {
  const auto& data = element.data<ButtonData>();
  const auto& style = data.style;

  Color bg_color;
  if (data.down) {
    bg_color = style.bg_color.multiply(style.input.active_color_factor);
  } else if (element->hovered) {
    bg_color = style.bg_color.multiply(style.input.hover_color_factor);
  } else {
    bg_color = style.bg_color;
  }

  Color border_color;
  if (element->in_focus_tree) {
    border_color = style.border_color.multiply(style.input.focus_color_factor);
  } else {
    border_color = style.border_color;
  }

  res.geometry_renderer.queue_box(
      element->box(),
      bg_color,
      style.border_width,
      border_color,
      style.radius);

  Vecf text_position =
      element->position + style.border_width.offset() + style.padding.offset();

  res.text_renderer
      .queue_text(text_position, data.text, style.text, style.width);
}

void ButtonSystem::mouse_event(Element element, const MouseEvent& event) {
  auto& data = element.data<ButtonData>();

  if (event.button != MouseButton::Left) {
    return;
  }
  switch (event.action) {
  case MouseAction::Press:
    data.down = true;
    break;
  case MouseAction::Release:
    data.down = false;
    if (element->box().contains(event.position)) {
      data.released = true;
      element.trigger();
    }
    break;
  default:
    break;
  }
}

void ButtonSystem::key_event(Element element, const KeyEvent& event) {
  auto& data = element.data<ButtonData>();
  if (event.key != Key::Enter) {
    return;
  }
  switch (event.action) {
  case KeyAction::Press:
    data.down = true;
    break;
  case KeyAction::Release:
    data.down = false;
    data.released = true;
    element.trigger();
    break;
  default:
    break;
  }
}

} // namespace datagui
