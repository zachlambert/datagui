#include "datagui/element/button.hpp"

namespace datagui {

void ButtonSystem::set_layout_input(Tree::Ptr node) const {
  const auto& element = elements[node->element_index];

  node->fixed_size =
      (element.style.border_width + element.style.padding).size();

  Vecf text_size = font_manager.text_size(element.text, element.style);
  node->fixed_size.y += text_size.y;

  if (auto width = std::get_if<LengthFixed>(&element.style.text_width)) {
    node->fixed_size.x += width->value;
  } else {
    node->fixed_size.x += text_size.x;
    if (auto width = std::get_if<LengthDynamic>(&element.style.text_width)) {
      node->dynamic_size.x = width->weight;
    }
  }
}

void ButtonSystem::render(Tree::ConstPtr node) const {
  const auto& element = elements[node->element_index];
  const auto& style = element.style;

  geometry_renderer.queue_box(
      node->box(),
      element.down ? style.down_color : style.bg_color,
      style.border_width,
      node->in_focus_tree ? style.focus_color : style.border_color,
      element.style.radius);

  Vecf text_position =
      node->position + (style.border_width + style.padding).offset();

  text_renderer.queue_text(text_position, element.text, style);
}

void ButtonSystem::mouse_event(Tree::Ptr node, const MouseEvent& event) {
  auto& element = elements[node->element_index];
  if (event.button != MouseButton::Left) {
    return;
  }
  switch (event.action) {
  case MouseAction::Press:
    element.down = true;
    break;
  case MouseAction::Release:
    element.down = false;
    element.released = true;
    node.trigger();
    break;
  default:
    break;
  }
}

void ButtonSystem::key_event(Tree::Ptr node, const KeyEvent& event) {
  auto& element = elements[node->element_index];
  if (event.key != Key::Enter) {
    return;
  }
  switch (event.action) {
  case KeyAction::Press:
    element.down = true;
    break;
  case KeyAction::Release:
    element.down = false;
    element.released = true;
    node.trigger();
    break;
  default:
    break;
  }
}

} // namespace datagui
