#include "datagui/element/horizontal_layout.hpp"

namespace datagui {

void HorizontalLayoutSystem::set_layout_input(Tree::Ptr node) const {
  auto& element = elements[node->element_index];
  const auto& style = element.style;

  node->fixed_size = Vecf::Zero();
  node->dynamic_size = Vecf::Zero();

  // Primary direction (X)

  if (auto length = std::get_if<LengthFixed>(&element.style.length)) {
    node->fixed_size.x = length->value;

  } else {
    auto child = node.first_child();
    int count = 0;
    while (child) {
      if (!child.visible()) {
        child = child.next();
        continue;
      }

      node->fixed_size.x += child->fixed_size.x;
      node->dynamic_size.x += child->dynamic_size.x;

      child = child.next();
      count++;
    }

    node->fixed_size.x += (count - 1) * element.style.inner_padding;

    if (auto length = std::get_if<LengthDynamic>(&element.style.length)) {
      node->dynamic_size.x = length->weight;
    }
  }

  // Secondary direction (Y)

  if (auto width = std::get_if<LengthFixed>(&element.style.width)) {
    node->fixed_size.y = width->value;
  } else {
    auto child = node.first_child();
    while (child) {
      if (!child.visible()) {
        child = child.next();
        continue;
      }

      node->fixed_size.y = std::max(node->fixed_size.y, child->fixed_size.y);
      node->dynamic_size.y =
          std::max(node->dynamic_size.y, child->dynamic_size.y);

      child = child.next();
    }

    if (auto width = std::get_if<LengthDynamic>(&element.style.width)) {
      node->dynamic_size.y = width->weight;
    }
  }

  node->fixed_size +=
      (element.style.outer_padding + element.style.border_width).size();
}

void HorizontalLayoutSystem::set_child_layout_output(Tree::Ptr node) const {
  const auto& element = elements[node->element_index];
  const auto& style = element.style;

  Vecf available = node->size - node->fixed_size;
  float offset_x = style.outer_padding.left + style.border_width.left;

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
      child->size.x +=
          (child->dynamic_size.x / children_dynamic_size.x) * available.x;
    }
    if (child->dynamic_size.y > 0) {
      child->size.y =
          node->size.y - (style.outer_padding + style.border_width).size().y;
    }

    child->position.x = node->position.x + offset_x;
    offset_x += child->size.x + style.inner_padding;

    switch (style.vertical_alignment) {
    case AlignmentY::Top:
      child->position.y =
          node->position.y + style.outer_padding.top + style.border_width.top;
      break;
    case AlignmentY::Center:
      child->position.y =
          node->position.y + node->size.y / 2 - child->size.y / 2;
      break;
    case AlignmentY::Bottom:
      child->position.y =
          node->position.y + node->size.y - child->size.y -
          (style.outer_padding.bottom + style.border_width.bottom);
      break;
    }

    child = child.next();
  }
}

void HorizontalLayoutSystem::render(Tree::ConstPtr node) const {
  const auto& element = elements[node->element_index];
  const auto& style = element.style;

  geometry_renderer.queue_box(
      Boxf(node->position, node->position + node->size),
      style.bg_color,
      style.border_width,
      style.border_color);
}

} // namespace datagui
