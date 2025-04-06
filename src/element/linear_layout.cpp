#include "datagui/element/linear_layout.hpp"

namespace datagui {

void LinearLayoutSystem::set_layout_input(Tree::Ptr node) const {
  auto& element = elements[node->element_index];
  const auto& style = element.style;

  node->fixed_size = Vecf::Zero();
  node->dynamic_size = Vecf::Zero();

  float& fixed_a = style.direction == Direction::Horizontal
                       ? node->fixed_size.x
                       : node->fixed_size.y;
  float& fixed_b = style.direction == Direction::Horizontal
                       ? node->fixed_size.y
                       : node->fixed_size.x;
  float& dynamic_a = style.direction == Direction::Horizontal
                         ? node->dynamic_size.x
                         : node->dynamic_size.y;
  float& dynamic_b = style.direction == Direction::Horizontal
                         ? node->dynamic_size.y
                         : node->dynamic_size.x;

  // Primary direction (a)

  if (auto length = std::get_if<LengthFixed>(&element.style.length)) {
    fixed_a = length->value;

  } else {
    auto child = node.first_child();
    int count = 0;
    while (child) {
      if (!child.visible()) {
        child = child.next();
        continue;
      }

      float child_fixed_a = style.direction == Direction::Horizontal
                                ? child->fixed_size.x
                                : child->fixed_size.y;
      float child_dynamic_a = style.direction == Direction::Horizontal
                                  ? child->dynamic_size.x
                                  : child->dynamic_size.y;
      float child_dynamic_b = style.direction == Direction::Horizontal
                                  ? child->dynamic_size.y
                                  : child->dynamic_size.x;

      fixed_a += child_fixed_a;
      dynamic_a += child_dynamic_a;

      child = child.next();
      count++;
    }
    fixed_a += (count - 1) * element.style.inner_padding;

    if (auto length = std::get_if<LengthDynamic>(&element.style.length)) {
      dynamic_a = length->weight;
    }
  }

  // Secondary direction (b)

  if (auto width = std::get_if<LengthFixed>(&element.style.width)) {
    fixed_b = width->value;
  } else {
    auto child = node.first_child();
    while (child) {
      if (!child.visible()) {
        child = child.next();
        continue;
      }

      float child_fixed_b = element.style.direction == Direction::Horizontal
                                ? child->fixed_size.y
                                : child->fixed_size.x;
      float child_dynamic_b = element.style.direction == Direction::Horizontal
                                  ? child->dynamic_size.y
                                  : child->dynamic_size.x;

      fixed_b = std::max(fixed_b, child_fixed_b);
      dynamic_b += std::max(dynamic_b, child_dynamic_b);

      child = child.next();
    }

    if (auto width = std::get_if<LengthDynamic>(&element.style.width)) {
      dynamic_b = width->weight;
    }
  }

  node->fixed_size +=
      (element.style.outer_padding + element.style.border_width).size();
}

void LinearLayoutSystem::set_child_layout_output(Tree::Ptr node) const {
  const auto& element = elements[node->element_index];
  const auto& style = element.style;

  Vecf available = node->size - node->fixed_size;
  Vecf offset = (style.outer_padding + style.border_width).offset();

  // Node dynamic size may differ from sum of child dynamic sizes, so need to
  // re-calculate
  Vecf children_dynamic_size = Vecf::Zero();
  {
    auto child = node.first_child();
    while (child) {
      children_dynamic_size += child->dynamic_size;
      child = child.next();
    }
  }

  auto child = node.first_child();
  while (child) {
    if (!child.visible()) {
      child = child.next();
      continue;
    }

    child->size = child->fixed_size;

    if (child->dynamic_size.x > 0) {
      if (style.direction == Direction::Horizontal) {
        child->size.x +=
            (child->dynamic_size.x / children_dynamic_size.x) * available.x;
      } else {
        child->size.x =
            node->size.x - (style.outer_padding + style.border_width).size().x;
      }
    }
    if (child->dynamic_size.y > 0) {
      if (style.direction == Direction::Horizontal) {
        child->size.y =
            node->size.y - (style.outer_padding + style.border_width).size().y;
      } else {
        child->size.y +=
            (child->dynamic_size.y / children_dynamic_size.y) * available.y;
      }
    }

    child->position = node->position + offset;

    if (style.direction == Direction::Horizontal) {
      offset.x += child->size.x + style.inner_padding;
    } else {
      offset.y += child->size.y + style.inner_padding;
    }
    child = child.next();
  }
}

void LinearLayoutSystem::render(Tree::ConstPtr node) const {
  const auto& element = elements[node->element_index];
  const auto& style = element.style;

  geometry_renderer.queue_box(
      Boxf(node->position, node->position + node->size),
      style.bg_color,
      style.border_width,
      style.border_color);
}

} // namespace datagui
