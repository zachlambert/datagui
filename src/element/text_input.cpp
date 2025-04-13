#include "datagui/element/text_input.hpp"

namespace datagui {

void TextInputSystem::set_layout_input(Tree::Ptr node) const {
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

void TextInputSystem::render(Tree::ConstPtr node) const {
  const auto& element = elements[node->element_index];
  const std::string& text = node == node->focused ? active_text : element.text;

  geometry_renderer.queue_box(
      node->box(),
      element.style.bg_color,
      element.style.border_width,
      node->in_focus_tree ? element.style.focus_color
                          : element.style.border_color,
      element.style.radius);

  Vecf text_position =
      node->position +
      (element.style.border_width + element.style.padding).offset();

  if (node->focused) {
    render_selection(
        font_manager.font_structure(
            element.style.font,
            element.style.font_size),
        element.style,
        text,
        text_position,
        active_selection,
        geometry_renderer);
  }

  text_renderer.queue_text(text_position, text, element.style);
}

void TextInputSystem::mouse_event(Tree::Ptr node, const MouseEvent& event) {
  const auto& element = elements[node->element_index];

  Vecf text_origin =
      node->position +
      (element.style.border_width + element.style.padding).offset();

  const auto& font =
      font_manager.font_structure(element.style.font, element.style.font_size);

  if (event.action == MouseAction::Press) {
    active_text = element.text;
  }

  std::size_t cursor_pos = find_cursor(
      font,
      active_text,
      element.style.text_width,
      event.position - text_origin);

  if (event.action == MouseAction::Press) {
    active_selection.reset(cursor_pos);
  } else if (event.action == MouseAction::Hold) {
    active_selection.end = cursor_pos;
  }
}

void TextInputSystem::key_event(Tree::Ptr node, const KeyEvent& event) {
  auto& element = elements[node->element_index];

  if (event.action == KeyAction::Press && event.key == Key::Enter) {
    element.text = active_text;
    element.changed = true;
    node.trigger();
    return;
  }
  selection_key_event(active_text, active_selection, true, event);
}

void TextInputSystem::text_event(Tree::Ptr node, const TextEvent& event) {
  selection_text_event(active_text, active_selection, true, event);
}

void TextInputSystem::focus_enter(Tree::Ptr node) {
  auto& element = elements[node->element_index];

  active_selection.reset(0);
  active_text = element.text;
}

void TextInputSystem::focus_leave(
    Tree::Ptr node,
    bool success,
    Tree::ConstPtr new_node) {

  auto& element = elements[node->element_index];
  if (success && element.text != active_text) {
    element.text = active_text;
    element.changed = true;
    node.trigger();
  }
}

} // namespace datagui
