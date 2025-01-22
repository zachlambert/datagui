#include "datagui/element/linear_layout.hpp"

namespace datagui {

void LinearLayoutSystem::calculate_size_components(Node& node, const Tree& tree)
    const {
  const auto& element = elements[node.element_index];

  node.fixed_size = Vecf::Zero();
  node.dynamic_size = Vecf::Zero();

  float& fixed_a = element.direction == LayoutDirection::Horizontal
                       ? node.fixed_size.x
                       : node.fixed_size.y;
  float& fixed_b = element.direction == LayoutDirection::Horizontal
                       ? node.fixed_size.y
                       : node.fixed_size.x;
  float& dynamic_a = element.direction == LayoutDirection::Horizontal
                         ? node.dynamic_size.x
                         : node.dynamic_size.y;
  float& dynamic_b = element.direction == LayoutDirection::Horizontal
                         ? node.dynamic_size.y
                         : node.dynamic_size.x;

  // Primary direction (a)

  if (element.length == 0) {
    int child_index = node.first_child;
    int count = 0;
    while (child_index != -1) {
      const auto& child = tree[child_index];

      if (child.hidden) {
        child_index = child.next;
        continue;
      }

      float child_fixed_a = element.direction == LayoutDirection::Horizontal
                                ? child.fixed_size.x
                                : child.fixed_size.y;
      float child_dynamic_a = element.direction == LayoutDirection::Horizontal
                                  ? child.dynamic_size.x
                                  : child.dynamic_size.y;

      fixed_a += child_fixed_a;
      dynamic_a += child_dynamic_a;

      child_index = child.next;
      count++;
    }
    fixed_a += (count - 1) * style.element.padding;

  } else if (element.length > 0) {
    fixed_a = element.length;
  } else {
    dynamic_a = -element.length;
  }

  // Secondary direction (b)

  if (element.width == 0) {
    int child_index = node.first_child;
    while (child_index != -1) {
      const auto& child = tree[child_index];
      if (child.hidden) {
        child_index = child.next;
        continue;
      }

      float child_fixed_b = element.direction == LayoutDirection::Horizontal
                                ? child.fixed_size.y
                                : child.fixed_size.x;
      float child_dynamic_b = element.direction == LayoutDirection::Horizontal
                                  ? child.dynamic_size.y
                                  : child.dynamic_size.x;

      fixed_b = std::max(fixed_b, child_fixed_b);
      dynamic_a += std::max(dynamic_b, child_dynamic_b);

      child_index = child.next;
    }

  } else if (element.width > 0) {
    fixed_a = element.width;
  } else {
    dynamic_a = -element.width;
  }

  node.fixed_size +=
      Vecf::Constant(2 * (style.element.border_width + style.element.padding));
}

void LinearLayoutSystem::calculate_child_dimensions(
    const Node& node,
    Tree& tree) const {
  const auto& element = elements[node.element_index];
  Vecf available = node.size - node.fixed_size;
  Vecf offset =
      Vecf::Constant(style.element.padding + style.element.border_width);

  int child_index = node.first_child;
  while (child_index != -1) {
    auto& child = tree[child_index];
    if (child.hidden) {
      child_index = child.next;
      continue;
    }

    child.size = child.fixed_size;

    if (child.dynamic_size.x > 0) {
      if (element.direction == LayoutDirection::Horizontal) {
        child.size.x +=
            (child.dynamic_size.x / node.dynamic_size.x) * available.x;
      } else {
        child.size.x = node.size.x -
                       2 * (style.element.padding + style.element.border_width);
      }
    }
    if (child.dynamic_size.y > 0) {
      if (element.direction == LayoutDirection::Horizontal) {
        child.size.y = node.size.y -
                       2 * (style.element.padding + style.element.border_width);
      } else {
        child.size.y +=
            (child.dynamic_size.y / node.dynamic_size.y) * available.y;
      }
    }

    child.origin = node.origin + offset;

    if (element.direction == LayoutDirection::Horizontal) {
      offset.x += child.size.x + style.element.padding;
    } else {
      offset.y += child.size.y + style.element.padding;
    }
    child_index = child.next;
  }
}

void LinearLayoutSystem::render(
    const Node& node,
    const NodeState& state,
    Renderers& renderers) const {
  const Color& border_color =
      state.focused ? style.element.focus_color : style.element.border_color;

  renderers.geometry.queue_box(
      Boxf(node.origin, node.origin + node.size),
      Color::Clear(),
      style.element.border_width,
      border_color,
      0);
}

} // namespace datagui
