#include "datagui/element/text_input.hpp"

namespace datagui {

void TextInputSystem::set_layout_input(Tree::Ptr node) const {
  const auto& element = elements[node->element_index];
  const std::string& text = *node.data<std::string>();

  node->fixed_size =
      (element.style.border_width + element.style.padding).size();

  Vecf text_size = font_manager.text_size(text, element.style);
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

  const std::string& text =
      node == active_node ? active_text : *node.data<std::string>();

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

  if (node == active_node) {
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
  const std::string& text = *node.data<std::string>();

  Vecf text_origin =
      node->position +
      (element.style.border_width + element.style.padding).offset();

  const auto& font =
      font_manager.font_structure(element.style.font, element.style.font_size);

  if (event.action == MouseAction::Press && active_node != node) {
    active_node = node;
    active_text = text;
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
    node.data<std::string>().mut() = active_text;
    return;
  }
  selection_key_event(active_text, active_selection, true, event);
}

void TextInputSystem::text_event(Tree::Ptr node, const TextEvent& event) {
  selection_text_event(active_text, active_selection, true, event);
}

void TextInputSystem::focus_enter(Tree::Ptr node) {
  if (active_node != node) {
    active_node = node;
    active_selection.reset(0);
  }
}

void TextInputSystem::focus_leave(
    Tree::Ptr node,
    bool success,
    Tree::ConstPtr new_node) {

  active_node = Tree::ConstPtr();
  if (!success) {
    return;
  }

  auto& element = elements[node->element_index];
  auto data = node.data<std::string>();
  if (*data != active_text) {
    data.mut() = active_text;
  }
}

} // namespace datagui
