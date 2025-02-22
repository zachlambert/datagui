#include "datagui/element/selection.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void SelectionSystem::calculate_size_components(Node& node, const Tree& tree) const {
  const auto& element = elements[node.element_index];
  node.fixed_size = Vecf::Constant(2 * (style.element.border_width + style.element.padding));

  const auto& group = groups[element.group];

  if (element.choice == -1) {
    node.fixed_size.y += font.line_height;
    if (group.max_width >= 0) {
      node.fixed_size.x += group.max_width;
    } else {
      node.dynamic_size.x = -group.max_width;
    }
  } else {
    node.fixed_size.y = font.line_height + 2 * (style.element.padding + style.element.border_width);
    node.dynamic_size.x = 1;
  }
}

void SelectionSystem::calculate_child_dimensions(const Node& node, Tree& tree) const {

  const auto& element = elements[node.element_index];
  if (element.choice != -1) {
    return;
  }

  Vecf offset = Vecf(0, node.fixed_size.y);

  int child_index = node.first_child;
  while (child_index != -1) {
    auto& child = tree[child_index];
    if (child.hidden) {
      child_index = child.next;
      continue;
    }

    child.size = child.fixed_size;

    if (child.dynamic_size.x > 0) {
      child.size.x = node.size.x;
    }

    child.floating = true;
    child.origin = node.origin + offset;
    offset.y += child.size.y;
    child_index = child.next;
  }
}

void SelectionSystem::render(const Node& node, const NodeState& state, Renderers& renderers) const {

  const auto& element = elements[node.element_index];
  const auto& group = groups[element.group];

  const Color& border_color =
      state.in_focus_tree ? style.element.focus_color : style.element.border_color;
  const Color& bg_color = element.choice != -1 && element.choice == group.choice
                              ? style.selection.choice_color
                              : style.element.bg_color;

  renderers.geometry.queue_box(
      Boxf(node.origin, node.origin + node.size),
      bg_color,
      style.element.border_width,
      border_color);

  int choice = element.choice == -1 ? group.choice : element.choice;
  if (choice >= 0 && choice < group.choices.size()) {
    renderers.text.queue_text(
        font,
        style.text.font_color,
        group.choices[choice],
        group.max_width,
        node.origin + Vecf::Constant(style.element.border_width + style.element.padding));
  }
}

bool SelectionSystem::release(const Node& node, const Vecf& mouse_pos) {
  const auto& element = elements[node.element_index];
  if (element.choice == -1) {
    return true;
  }
  auto& group = groups[element.group];
  if (group.choice != element.choice) {
    group.choice = element.choice;
    return true;
  }
  return false;
}

bool SelectionSystem::key_event(const Node& node, const KeyEvent& event) {
  const auto& element = elements[node.element_index];
  if (element.choice == -1) {
    return false;
  }
  if (!event.is_text && event.key_release && event.key_value == KeyValue::Enter) {
    auto& group = groups[element.group];
    if (group.choice != element.choice) {
      group.choice = element.choice;
      return true;
    }
  }
  return false;
}

} // namespace datagui
