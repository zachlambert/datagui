#include "datagui/element/button.hpp"

namespace datagui {

bool ButtonSystem::visit(Element element, const std::string& text) {
  auto& data = element.data<ButtonData>();
  if (element.rerender()) {
    data.text = text;
    data.style.apply(res.style_manager);
  }
  if (data.released) {
    data.released = false;
    return true;
  }
  return false;
}

void ButtonSystem::set_layout_input(Element element) const {
  const auto& data = element.data<ButtonData>();
  const auto& style = data.style;

  element->fixed_size = (style.box.border_width + style.box.padding).size();
  element->dynamic_size = Vecf::Zero();
  element->floating = false;

  Vecf text_size =
      res.font_manager.text_size(data.text, style.text, style.box.width);
  element->fixed_size.y += text_size.y;

  if (auto width = std::get_if<LengthFixed>(&style.box.width)) {
    element->fixed_size.x += width->value;
  } else {
    element->fixed_size.x += text_size.x;
    if (auto width = std::get_if<LengthDynamic>(&style.box.width)) {
      element->dynamic_size.x = width->weight;
    }
  }
}

void ButtonSystem::render(ConstElement element) const {
  const auto& data = element.data<ButtonData>();
  const auto& style = data.style;

  res.geometry_renderer.queue_box(
      element->box(),
      data.down          ? style.active_color
      : element->hovered ? style.hover_color
                         : style.box.bg_color,
      style.box.border_width,
      element->in_focus_tree ? style.focus_color : style.box.border_color,
      style.box.radius);

  Vecf text_position =
      element->position + (style.box.border_width + style.box.padding).offset();

  res.text_renderer
      .queue_text(text_position, data.text, style.text, style.box.width);
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
