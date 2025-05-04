#include "datagui/element/horizontal_layout.hpp"

namespace datagui {

void HorizontalLayoutSystem::visit(
    Element element,
    const SetHorizontalLayoutStyle& set_style) {
  auto& data = element.data<HorizontalLayoutData>();
  if (element.rerender()) {
    if (set_style) {
      set_style(data.style);
    }
  }
}

void HorizontalLayoutSystem::set_layout_input(Element element) const {
  auto& data = element.data<HorizontalLayoutData>();
  auto& style = data.style;

  element->fixed_size = Vecf::Zero();
  element->dynamic_size = Vecf::Zero();
  element->floating = false;

  // Primary direction (X)

  if (auto length = std::get_if<LengthFixed>(&style.length)) {
    element->fixed_size.x = length->value;

  } else {
    auto child = element.first_child();
    int count = 0;
    while (child) {
      if (!child.visible()) {
        child = child.next();
        continue;
      }

      element->fixed_size.x += child->fixed_size.x;
      element->dynamic_size.x += child->dynamic_size.x;

      child = child.next();
      count++;
    }

    element->fixed_size.x += (count - 1) * style.inner_padding;

    if (auto length = std::get_if<LengthDynamic>(&style.length)) {
      element->dynamic_size.x = length->weight;
    }
  }

  // Secondary direction (Y)

  if (auto width = std::get_if<LengthFixed>(&style.width)) {
    element->fixed_size.y = width->value;
  } else {
    auto child = element.first_child();
    while (child) {
      if (!child.visible()) {
        child = child.next();
        continue;
      }

      element->fixed_size.y =
          std::max(element->fixed_size.y, child->fixed_size.y);
      element->dynamic_size.y =
          std::max(element->dynamic_size.y, child->dynamic_size.y);

      child = child.next();
    }

    if (auto width = std::get_if<LengthDynamic>(&style.width)) {
      element->dynamic_size.y = width->weight;
    }
  }

  element->fixed_size += (style.padding + style.border_width).size();
}

void HorizontalLayoutSystem::set_child_layout_output(Element element) const {
  const auto& data = element.data<HorizontalLayoutData>();
  const auto& style = data.style;

  Vecf available = element->size - element->fixed_size;
  float offset_x = style.padding.left + style.border_width.left;

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

    if (child->dynamic_size.x > 0) {
      child->size.x +=
          (child->dynamic_size.x / children_dynamic_size.x) * available.x;
    }
    if (child->dynamic_size.y > 0) {
      child->size.y =
          element->size.y - (style.padding + style.border_width).size().y;
    }

    child->position.x = element->position.x + offset_x;
    offset_x += child->size.x + style.inner_padding;

    switch (style.vertical_alignment) {
    case AlignmentY::Top:
      child->position.y =
          element->position.y + style.padding.top + style.border_width.top;
      break;
    case AlignmentY::Center:
      child->position.y =
          element->position.y + element->size.y / 2 - child->size.y / 2;
      break;
    case AlignmentY::Bottom:
      child->position.y = element->position.y + element->size.y -
                          child->size.y -
                          (style.padding.bottom + style.border_width.bottom);
      break;
    }

    child = child.next();
  }
}

void HorizontalLayoutSystem::render(ConstElement element) const {
  const auto& data = element.data<HorizontalLayoutData>();
  const auto& style = data.style;

  geometry_renderer.queue_box(element->box(), style);
}

} // namespace datagui
