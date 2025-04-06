#include "datagui/element/linear_layout.hpp"

namespace datagui {

void LinearLayoutSystem::set_layout_input(Tree::Ptr node) const {
  const auto& element = elements[node->element_index];
  const auto& style = element.style;

  node->fixed_size = Vecf::Zero();
  node->dynamic_size = Vecf::Zero();

  float& fixed_a = style.direction == LinearLayoutDirection::Horizontal
                       ? node->fixed_size.x
                       : node->fixed_size.y;
  float& fixed_b = style.direction == LinearLayoutDirection::Horizontal
                       ? node->fixed_size.y
                       : node->fixed_size.x;
  float& dynamic_a = style.direction == LinearLayoutDirection::Horizontal
                         ? node->dynamic_size.x
                         : node->dynamic_size.y;
  float& dynamic_b = style.direction == LinearLayoutDirection::Horizontal
                         ? node->dynamic_size.y
                         : node->dynamic_size.x;

  // Primary direction (a)

  if (style.length == 0) {
    auto child = node.first_child();
    int count = 0;
    while (child) {
      if (!child.visible()) {
        child = child.next();
        continue;
      }

      float child_fixed_a = style.direction == LinearLayoutDirection::Horizontal
                                ? child->fixed_size.x
                                : child->fixed_size.y;
      float child_dynamic_a =
          style.direction == LinearLayoutDirection::Horizontal
              ? child->dynamic_size.x
              : child->dynamic_size.y;

      fixed_a += child_fixed_a;
      dynamic_a += child_dynamic_a;

      child = child.next();
      count++;
    }
    fixed_a += (count - 1) * element.style.inner_padding;

  } else if (element.style.length > 0) {
    fixed_a = element.style.length;
  } else {
    dynamic_a = -element.style.length;
  }

  // Secondary direction (b)

  if (element.style.width == 0) {
    auto child = node.first_child();
    while (child) {
      if (!child.visible()) {
        child = child.next();
        continue;
      }

      float child_fixed_b =
          element.style.direction == LinearLayoutDirection::Horizontal
              ? child->fixed_size.y
              : child->fixed_size.x;
      float child_dynamic_b =
          element.style.direction == LinearLayoutDirection::Horizontal
              ? child->dynamic_size.y
              : child->dynamic_size.x;

      fixed_b = std::max(fixed_b, child_fixed_b);
      dynamic_b += std::max(dynamic_b, child_dynamic_b);

      child = child.next();
    }

  } else if (element.style.width > 0) {
    fixed_a = element.style.width;
  } else {
    dynamic_a = -element.style.width;
  }

  node->fixed_size +=
      Vecf::Constant(2 * (element.style.border_width + element.style.padding));
}

void LinearLayoutSystem::set_child_layout_output(Tree::Ptr node) const {
  const auto& element = elements[node->element_index];
  const auto& style = element.style;

  Vecf available = node->size - node->fixed_size;
  Vecf offset = Vecf::Constant(style.padding + style.border_width);

  auto child = node.first_child();
  while (child) {
    if (!child.visible()) {
      child = child.next();
      continue;
    }

    child->size = child->fixed_size;

    if (child->dynamic_size.x > 0) {
      if (style.direction == LinearLayoutDirection::Horizontal) {
        child->size.x +=
            (child->dynamic_size.x / node->dynamic_size.x) * available.x;
      } else {
        child->size.x = node->size.x - 2 * (style.padding + style.border_width);
      }
    }
    if (child->dynamic_size.y > 0) {
      if (style.direction == LinearLayoutDirection::Horizontal) {
        child->size.y = node->size.y - 2 * (style.padding + style.border_width);
      } else {
        child->size.y +=
            (child->dynamic_size.y / node->dynamic_size.y) * available.y;
      }
    }

    child->position = node->position + offset;

    if (style.direction == LinearLayoutDirection::Horizontal) {
      offset.x += child->size.x + style.padding;
    } else {
      offset.y += child->size.y + style.padding;
    }
    child = child.next();
  }
}

void LinearLayoutSystem::render(Tree::ConstPtr node) const {
  const auto& element = elements[node->element_index];
  const auto& style = element.style;
#if 0
  const Color& border_color = state.in_focus_tree ? style.element.focus_color
                                                  : style.element.border_color;

  renderers.geometry.queue_box(
      Boxf(node.origin, node.origin + node.size),
      Color::Clear(),
      style.element.border_width,
      border_color,
      0);
#endif

  geometry_renderer.queue_box(
      Boxf(node->position, node->position + node->size),
      style.bg_color,
      style.border_width,
      style.border_color,
      style.border_width);
}

} // namespace datagui
