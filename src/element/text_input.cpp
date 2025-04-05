#include "datagui/element/text_input.hpp"

namespace datagui {

void TextInputSystem::set_layout_input(Tree::Ptr node) const {
  const auto& element = elements[node->element_index];
  const std::string& text = *node.data<std::string>();

  node->fixed_size =
      Vecf::Constant(2 * (element.style.border_width + element.style.padding));

  Vecf text_size = font_manager.text_size(
      element.style.font,
      element.style.font_size,
      text,
      element.style.max_width);

  if (element.style.max_width >= 0) {
    node->fixed_size += text_size;
  } else {
    node->fixed_size.y += text_size.y;
    node->dynamic_size.x = -1;
  }
}

void TextInputSystem::render(Tree::ConstPtr node) const {
  const auto& element = elements[node->element_index];

  const std::string& text =
      node == active_node ? active_text : *node.data<std::string>();

  geometry_renderer.queue_box(
      Boxf(node->position, node->position + node->size),
      element.style.bg_color,
      element.style.border_width,
      node->in_focus_tree ? element.style.focus_color
                          : element.style.border_color);
  Vecf text_position =
      node->position +
      Vecf::Constant(element.style.border_width + element.style.padding);

  if (node == active_node) {
    TextSelectionStyle style;
    style.cursor_color = Color::Gray(0.3);
    style.cursor_width = 2;
    style.max_width = element.style.max_width;
    style.disabled = false;
    style.highlight_color = Color::Gray(0.5);
    render_selection(
        font_manager.font_structure(
            element.style.font,
            element.style.font_size),
        style,
        text,
        text_position,
        active_selection,
        geometry_renderer);
  }

  text_renderer.queue_text(
      text,
      text_position,
      element.style.font,
      element.style.font_size,
      element.style.text_color,
      element.style.max_width);
}

void TextInputSystem::mouse_event(Tree::Ptr node, const MouseEvent& event) {
  const auto& element = elements[node->element_index];
  const std::string& text = *node.data<std::string>();

  Vecf text_origin =
      node->position +
      Vecf::Constant(element.style.border_width + element.style.padding);

  const auto& font =
      font_manager.font_structure(element.style.font, element.style.font_size);

  std::size_t cursor_pos = find_cursor(
      font,
      text,
      element.style.max_width,
      event.position - text_origin);

  if (event.action == MouseAction::Press) {
    active_node = node;
    active_text = text;
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
  active_node = node;
  active_selection.reset(0);
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
