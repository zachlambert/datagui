#include "datagui/element/text_input.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void TextInputSystem::calculate_size_components(Node& node, const Tree& tree) const {
  const auto& element = elements[node.element_index];

  node.fixed_size = Vecf::Constant(2 * (style.element.border_width + style.element.padding));

  if (element.max_width >= 0) {
    node.fixed_size += text_size(font, *element.text, element.max_width);
  } else {
    node.fixed_size.y += font.line_height;
    node.dynamic_size.x = -element.max_width;
  }
}

void TextInputSystem::render(const Node& node, const NodeState& state, Renderers& renderers) const {
  const auto& element = elements[node.element_index];

  renderers.geometry.queue_box(
      Boxf(node.origin, node.origin + node.size),
      style.text_input.bg_color,
      style.element.border_width,
      state.in_focus_tree ? style.element.focus_color : style.element.border_color);
  Vecf text_origin =
      node.origin + Vecf::Constant(style.element.border_width + style.element.padding);

  if (state.focused) {
    render_selection(
        style,
        font,
        *element.text,
        element.max_width,
        text_origin,
        text_selection,
        true,
        renderers.geometry);
  }

  renderers.text
      .queue_text(font, style.text.font_color, *element.text, element.max_width, text_origin);
}

bool TextInputSystem::press(const Node& node, const Vecf& mouse_pos) {
  const auto& element = elements[node.element_index];
  Vecf text_origin =
      node.origin + Vecf::Constant(style.element.border_width + style.element.padding);
  text_selection.reset(
      find_cursor(font, *element.text, element.max_width, mouse_pos - text_origin));
  return false;
}

bool TextInputSystem::held(const Node& node, const Vecf& mouse_pos) {
  const auto& element = elements[node.element_index];
  Vecf text_origin =
      node.origin + Vecf::Constant(style.element.border_width + style.element.padding);
  text_selection.end = find_cursor(font, *element.text, element.max_width, mouse_pos - text_origin);
  return false;
}

bool TextInputSystem::focus_enter(const Node& node) {
  text_selection.reset(0);
  return false;
}

bool TextInputSystem::focus_leave(const Tree& tree, const Node& node, bool success, int new_focus) {
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

  if (!event.is_text && !event.key_release && event.key_value == KeyValue::Enter) {
    if (element.initial_text != *element.text) {
      element.initial_text = *element.text;
      return true;
    }
  } else {
    selection_key_event(*element.text, text_selection, true, event);
  }
  return false;
}

} // namespace datagui
