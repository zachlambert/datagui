#include "datagui/element/selection.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void SelectionSystem::set_choice(const Node& node, int choice) {
  elements[node.element_index].choice = choice;
}

void SelectionSystem::calculate_size_components(Node& node, const Tree& tree) const {
  const auto& element = elements[node.element_index];
  node.fixed_size = Vecf::Constant(2 * (style.element.border_width + style.element.padding));

  node.fixed_size.y += font.line_height;
  if (element.max_width >= 0) {
    node.fixed_size.x += element.max_width;
  } else {
    node.dynamic_size.x = -element.max_width;
  }
}

void SelectionSystem::calculate_child_dimensions(const Node& node, Tree& tree) const {

  const auto& element = elements[node.element_index];
  Vecf available = node.size - node.fixed_size;
  Vecf offset = Vecf::Constant(style.element.padding + style.element.border_width);

  int child_index = node.first_child;
  while (child_index != -1) {
    auto& child = tree[child_index];
    if (child.hidden) {
      child_index = child.next;
      continue;
    }

    child.size = child.fixed_size;

    if (child.dynamic_size.x > 0) {
      child.size.x = node.size.x - 2 * (style.element.padding + style.element.border_width);
    }
    if (child.dynamic_size.y > 0) {
      child.size.y += (child.dynamic_size.y / node.dynamic_size.y) * available.y;
    }

    child.floating = true;
    child.origin = node.origin + offset;
    offset.y += child.size.y + style.element.padding;
    child_index = child.next;
  }
}

void SelectionSystem::render(const Node& node, const NodeState& state, Renderers& renderers) const {
  const auto& element = elements[node.element_index];
  const Color& border_color =
      state.focused ? style.element.focus_color : style.element.border_color;

  renderers.geometry.queue_box(
      Boxf(node.origin, node.origin + node.size),
      style.element.bg_color,
      style.element.border_width,
      border_color);
  if (element.choice >= 0 && element.choice < element.choices.size()) {
    renderers.text.queue_text(
        font,
        style.text.font_color,
        element.choices[element.choice],
        element.max_width,
        node.origin + Vecf::Constant(style.element.border_width + style.element.padding));
  }
}

void OptionSystem::calculate_size_components(Node& node, const Tree& tree) const {
  const auto& element = elements[node.element_index];
  node.fixed_size = text_size(font, element.choice, element.max_width);
  node.fixed_size += Vecf::Constant(2 * (style.element.border_width + style.element.padding));
}

void OptionSystem::render(const Node& node, const NodeState& state, Renderers& renderers) const {

  const auto& element = elements[node.element_index];
  const Color& bg_color = state.held ? style.element.pressed_bg_color : style.element.bg_color;
  const Color& border_color =
      state.focused ? style.element.focus_color : style.element.border_color;

  renderers.geometry.queue_box(
      Boxf(node.origin, node.origin + node.size),
      bg_color,
      style.element.border_width,
      border_color);
  renderers.text.queue_text(
      font,
      style.text.font_color,
      element.choice,
      element.max_width,
      node.origin + Vecf::Constant(style.element.border_width + style.element.padding));
}

} // namespace datagui
