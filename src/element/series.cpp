#include "datagui/element/series.hpp"

namespace datagui {

void SeriesSystem::visit(Element element) {
  auto& data = element.data<SeriesData>();
  if (element.rerender()) {
    data.style.apply(res.style_manager);
  }
}

void SeriesSystem::set_layout_input(Element element) const {
  auto& data = element.data<SeriesData>();
  const auto& style = data.style;

  element->fixed_size = Vecf::Zero();
  element->dynamic_size = Vecf::Zero();
  element->floating = false;

  // Primary direction

  {
    auto child = element.first_child();
    int count = 0;
    while (child) {
      if (!child.visible() || child->zero_size()) {
        child = child.next();
        continue;
      }

      if (style.direction == Direction::Horizontal) {
        element->fixed_size.x += child->fixed_size.x;
        element->dynamic_size.x += child->dynamic_size.x;
      } else {
        element->fixed_size.y += child->fixed_size.y;
        element->dynamic_size.y += child->dynamic_size.y;
      }

      child = child.next();
      count++;
    }

    if (style.direction == Direction::Horizontal) {
      element->fixed_size.x += (count - 1) * style.inner_padding;
    } else {
      element->fixed_size.y += (count - 1) * style.inner_padding;
    }
  }
  if (auto length = std::get_if<LengthFixed>(&style.length)) {
    if (style.direction == Direction::Horizontal) {
      data.overrun = std::max(element->fixed_size.x - length->value, 0.f);
      element->fixed_size.x = length->value;
    } else {
      data.overrun = std::max(element->fixed_size.y - length->value, 0.f);
      element->fixed_size.y = length->value;
    }
  } else if (auto length = std::get_if<LengthDynamic>(&style.length)) {
    if (style.direction == Direction::Horizontal) {
      element->dynamic_size.y = length->weight;
    } else {
      element->dynamic_size.x = length->weight;
    }
  }

  // Secondary direction

  if (auto width = std::get_if<LengthFixed>(&style.width)) {
    if (style.direction == Direction::Horizontal) {
      element->fixed_size.y = width->value;
    } else {
      element->fixed_size.x = width->value;
    }
  } else {
    auto child = element.first_child();
    while (child) {
      if (!child.visible() || child->zero_size()) {
        child = child.next();
        continue;
      }

      if (style.direction == Direction::Horizontal) {
        element->fixed_size.y =
            std::max(element->fixed_size.y, child->fixed_size.y);
        element->dynamic_size.y =
            std::max(element->dynamic_size.y, child->dynamic_size.y);
      } else {
        element->fixed_size.x =
            std::max(element->fixed_size.x, child->fixed_size.x);
        element->dynamic_size.x =
            std::max(element->dynamic_size.x, child->dynamic_size.x);
      }

      child = child.next();
    }

    if (auto width = std::get_if<LengthDynamic>(&style.width)) {
      if (style.direction == Direction::Horizontal) {
        element->dynamic_size.y = width->weight;
      } else {
        element->dynamic_size.x = width->weight;
      }
    }
  }

  element->fixed_size += style.outer_padding.size();
}

void SeriesSystem::set_child_layout_output(Element element) const {
  const auto& data = element.data<SeriesData>();
  const auto& style = data.style;

  Vecf available = minimum(element->size - element->fixed_size, Vecf::Zero());
  float offset;
  if (style.direction == Direction::Horizontal) {
    offset = style.outer_padding.left - data.scroll_pos;
  } else {
    offset = style.outer_padding.top - data.scroll_pos;
  }

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
    if (!child.visible() || child->zero_size()) {
      child = child.next();
      continue;
    }

    child->size = child->fixed_size;

    if (style.direction == Direction::Horizontal) {
      if (child->dynamic_size.x > 0) {
        child->size.x +=
            (child->dynamic_size.x / children_dynamic_size.x) * available.x;
      }
      if (child->dynamic_size.y > 0) {
        child->size.y = element->size.y - style.outer_padding.size().y;
      }

      child->position.x = element->position.x + offset;
      offset += child->size.x + style.inner_padding;

      switch (style.alignment) {
      case Alignment::Min:
        child->position.y = element->position.y + style.outer_padding.top;
        break;
      case Alignment::Center:
        child->position.y =
            element->position.y + element->size.y / 2 - child->size.y / 2;
        break;
      case Alignment::Max:
        child->position.y = element->position.y + element->size.y -
                            child->size.y - style.outer_padding.bottom;
        break;
      default:
        assert(false);
        break;
      }

    } else {
      if (child->dynamic_size.y > 0) {
        child->size.y +=
            (child->dynamic_size.y / children_dynamic_size.y) * available.y;
      }
      if (child->dynamic_size.x > 0) {
        child->size.x = element->size.x - style.outer_padding.size().x;
      }

      child->position.y = element->position.y + offset;
      offset += child->size.y + style.inner_padding;

      switch (style.alignment) {
      case Alignment::Min:
        child->position.x = element->position.x + style.outer_padding.left;
        break;
      case Alignment::Center:
        child->position.x =
            element->position.x + element->size.x / 2 - child->size.x / 2;
        break;
      case Alignment::Max:
        child->position.x = element->position.x + element->size.x -
                            child->size.x - style.outer_padding.right;
        break;
      default:
        assert(false);
        break;
      }
    }

    child = child.next();
  }
}

void SeriesSystem::render(ConstElement element) const {
  const auto& data = element.data<SeriesData>();
  const auto& style = data.style;

  res.geometry_renderer
      .queue_box(element->box(), style.bg_color, 0, Color::Black(), 0);
}

bool SeriesSystem::scroll_event(Element element, const ScrollEvent& event) {
  auto& data = element.data<SeriesData>();

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
