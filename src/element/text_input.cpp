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
  const std::string& text = *node.data<std::string>();

  geometry_renderer.queue_box(
      Boxf(node->position, node->position + node->size),
      element.style.bg_color,
      element.style.border_width,
      node->in_focus_tree ? element.style.focus_color
                          : element.style.border_color);
  Vecf text_position =
      node->position +
      Vecf::Constant(element.style.border_width + element.style.padding);

  if (node->focused) {
#if 0
    render_selection(
        style,
        font,
        *element.text,
        element.max_width,
        text_origin,
        text_selection,
        true,
        renderers.geometry);
#endif
  }

  text_renderer.queue_text(
      text,
      text_position,
      element.style.font,
      element.style.font_size,
      element.style.text_color,
      element.style.max_width);
}

#if 0
bool TextInputSystem::press(const Node& node, const Vecf& mouse_pos) {
  const auto& element = elements[node.element_index];
  Vecf text_origin =
      node.origin +
      Vecf::Constant(style.element.border_width + style.element.padding);
  text_selection.reset(find_cursor(
      font,
      *element.text,
      element.max_width,
      mouse_pos - text_origin));
  return false;
}

bool TextInputSystem::held(const Node& node, const Vecf& mouse_pos) {
  const auto& element = elements[node.element_index];
  Vecf text_origin =
      node.origin +
      Vecf::Constant(style.element.border_width + style.element.padding);
  text_selection.end = find_cursor(
      font,
      *element.text,
      element.max_width,
      mouse_pos - text_origin);
  return false;
}

bool TextInputSystem::focus_enter(const Node& node) {
  text_selection.reset(0);
  return false;
}

bool TextInputSystem::focus_leave(
    const Tree& tree,
    const Node& node,
    bool success,
    int new_focus) {
  auto& element = elements[node.element_index];
  if (!success) {
    *element.text = element.initial_text;
  } else if (element.initial_text != *element.text) {
    element.initial_text = *element.text;
    // TODO: Fix so don't need const cast
    element.text.mutate(const_cast<Tree&>(tree));
    return true;
  }
  return false;
}

bool TextInputSystem::key_event(const Node& node, const KeyEvent& event) {
  auto& element = elements[node.element_index];

  if (!event.is_text && !event.key_release &&
      event.key_value == KeyValue::Enter) {
    if (element.initial_text != *element.text) {
      element.initial_text = *element.text;
      return true;
    }
  } else {
    selection_key_event(*element.text, text_selection, true, event);
  }
  return false;
}
#endif

} // namespace datagui
