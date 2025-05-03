#include "datagui/element/vertical_layout.hpp"

namespace datagui {

void VerticalLayoutSystem::visit(
    Element element,
    const SetVerticalLayoutStyle& set_style) {
  if (element.rerender()) {
    if (set_style) {
      set_style(element.data<VerticalLayoutData>().style);
    }
  }
}

void VerticalLayoutSystem::set_layout_input(Element element) const {
  auto& data = element.data<VerticalLayoutData>();
  const auto& style = data.style;

  element->fixed_size = Vecf::Zero();
  element->dynamic_size = Vecf::Zero();
  element->layer_offset = 0;

  // Primary direction (Y)

  {
    auto child = element.first_child();
    int count = 0;
    while (child) {
      if (!child.visible()) {
        child = child.next();
        continue;
      }

      element->fixed_size.y += child->fixed_size.y;
      element->dynamic_size.y += child->dynamic_size.y;

      child = child.next();
      count++;
    }

    element->fixed_size.y += (count - 1) * style.inner_padding;
  }
  if (auto length = std::get_if<LengthFixed>(&style.length)) {
    data.overrun = std::max(element->fixed_size.y - length->value, 0.f);
    element->fixed_size.y = length->value;
  } else if (auto length = std::get_if<LengthDynamic>(&style.length)) {
    element->dynamic_size.y = length->weight;
  }

  // Secondary direction (X)

  if (auto width = std::get_if<LengthFixed>(&style.width)) {
    element->fixed_size.x = width->value;
  } else {
    auto child = element.first_child();
    while (child) {
      if (!child.visible()) {
        child = child.next();
        continue;
      }

      element->fixed_size.x =
          std::max(element->fixed_size.x, child->fixed_size.x);
      element->dynamic_size.x =
          std::max(element->dynamic_size.x, child->dynamic_size.x);

      child = child.next();
    }

    if (auto width = std::get_if<LengthDynamic>(&style.width)) {
      element->dynamic_size.x = width->weight;
    }
  }

  element->fixed_size += (style.padding + style.border_width).size();
}

void VerticalLayoutSystem::set_child_layout_output(Element element) const {
  const auto& data = element.data<VerticalLayoutData>();
  const auto& style = data.style;

  Vecf available = minimum(element->size - element->fixed_size, Vecf::Zero());
  float offset_y = style.padding.top + style.border_width.top - data.scroll_pos;

  // Node dynamic size may differ from sum of child dynamic sizes, so need to
  // re-calculate
  Vecf children_dynamic_size = Vecf::Zero();
  {
    auto child = element.first_child();
    while (child) {
      children_dynamic_size += child->dynamic_size;
      child = child.next();
    }
  }

  auto child = element.first_child();
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
          element->size.x - (style.padding + style.border_width).size().x;
    }

    child->position.y = element->position.y + offset_y;
    offset_y += child->size.y + style.inner_padding;

    switch (style.horizontal_alignment) {
    case AlignmentX::Left:
      child->position.x =
          element->position.x + style.padding.left + style.border_width.left;
      break;
    case AlignmentX::Center:
      child->position.x =
          element->position.x + element->size.x / 2 - child->size.x / 2;
      break;
    case AlignmentX::Right:
      child->position.x = element->position.x + element->size.x -
                          child->size.x -
                          (style.padding.right + style.border_width.right);
      break;
    }

    child = child.next();
  }
}

void VerticalLayoutSystem::render(ConstElement element) const {
  const auto& data = element.data<VerticalLayoutData>();
  const auto& style = data.style;

  geometry_renderer.queue_box(element->box(), style);
}

bool VerticalLayoutSystem::scroll_event(
    Element element,
    const ScrollEvent& event) {

  auto& data = element.data<VerticalLayoutData>();

  if (data.overrun == 0) {
    return false;
  }
  if (data.scroll_pos == data.overrun && event.amount > 0 ||
      data.scroll_pos == 0 && event.amount < 0) {
    return false;
  }

  data.scroll_pos =
      std::clamp(data.scroll_pos + event.amount, 0.f, data.overrun);
  return true;
}

} // namespace datagui
