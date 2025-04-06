#include "datagui/element/vertical_layout.hpp"

namespace datagui {

void VerticalLayoutSystem::set_layout_input(Tree::Ptr node) const {
  auto& element = elements[node->element_index];
  const auto& style = element.style;

  node->fixed_size = Vecf::Zero();
  node->dynamic_size = Vecf::Zero();

  // Primary direction (Y)

  if (auto length = std::get_if<LengthFixed>(&element.style.length)) {
    node->fixed_size.y = length->value;

  } else {
    auto child = node.first_child();
    int count = 0;
    while (child) {
      if (!child.visible()) {
        child = child.next();
        continue;
      }

      node->fixed_size.y += child->fixed_size.y;
      node->dynamic_size.y += child->dynamic_size.y;

      child = child.next();
      count++;
    }

    node->fixed_size.y += (count - 1) * element.style.inner_padding;

    if (auto length = std::get_if<LengthDynamic>(&element.style.length)) {
      node->dynamic_size.y = length->weight;
    }
  }

  // Secondary direction (X)

  if (auto width = std::get_if<LengthFixed>(&element.style.width)) {
    node->fixed_size.x = width->value;
  } else {
    auto child = node.first_child();
    while (child) {
      if (!child.visible()) {
        child = child.next();
        continue;
      }

      node->fixed_size.x = std::max(node->fixed_size.x, child->fixed_size.x);
      node->dynamic_size.x =
          std::max(node->dynamic_size.x, child->dynamic_size.x);

      child = child.next();
    }

    if (auto width = std::get_if<LengthDynamic>(&element.style.width)) {
      node->dynamic_size.x = width->weight;
    }
  }

  node->fixed_size +=
      (element.style.outer_padding + element.style.border_width).size();
}

void VerticalLayoutSystem::set_child_layout_output(Tree::Ptr node) const {
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

    if (child->dynamic_size.y > 0) {
      child->size.y +=
          (child->dynamic_size.y / children_dynamic_size.y) * available.y;
    }
    if (child->dynamic_size.x > 0) {
      child->size.x =
          node->size.x - (style.outer_padding + style.border_width).size().x;
    }

    child->position = node->position + offset;
    offset.y += child->size.y + style.inner_padding;

    child = child.next();
  }
}

void VerticalLayoutSystem::render(Tree::ConstPtr node) const {
  const auto& element = elements[node->element_index];
  const auto& style = element.style;

  geometry_renderer.queue_box(
      Boxf(node->position, node->position + node->size),
      style.bg_color,
      style.border_width,
      style.border_color);
}

} // namespace datagui
