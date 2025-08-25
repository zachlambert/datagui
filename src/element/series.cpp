#include "datagui/element/series.hpp"

namespace datagui {

void SeriesElement::set_input_state(
    const std::vector<const Element*>& children) {
  fixed_size = Vecf::Zero();
  dynamic_size = Vecf::Zero();
  floating = false;

  // Primary direction

  {
    for (auto child : children) {
      if (child->hidden || child->zero_size()) {
        continue;
      }

      if (style.direction == Direction::Horizontal) {
        fixed_size.x += child->fixed_size.x;
        dynamic_size.x += child->dynamic_size.x;
      } else {
        fixed_size.y += child->fixed_size.y;
        dynamic_size.y += child->dynamic_size.y;
      }
    }

    if (!children.empty()) {
      if (style.direction == Direction::Horizontal) {
        fixed_size.x += (children.size() - 1) * style.inner_padding;
      } else {
        fixed_size.y += (children.size() - 1) * style.inner_padding;
      }
    }
  }
  if (auto length = std::get_if<LengthFixed>(&style.length)) {
    if (style.direction == Direction::Horizontal) {
      overrun = std::max(fixed_size.x - length->value, 0.f);
      fixed_size.x = length->value;
    } else {
      overrun = std::max(fixed_size.y - length->value, 0.f);
      fixed_size.y = length->value;
    }
  } else if (auto length = std::get_if<LengthDynamic>(&style.length)) {
    if (style.direction == Direction::Horizontal) {
      dynamic_size.y = length->weight;
    } else {
      dynamic_size.x = length->weight;
    }
  }

  // Secondary direction

  if (auto width = std::get_if<LengthFixed>(&style.width)) {
    if (style.direction == Direction::Horizontal) {
      fixed_size.y = width->value;
    } else {
      fixed_size.x = width->value;
    }
  } else {
    for (auto child : children) {
      if (child->hidden || child->zero_size()) {
        continue;
      }

      if (style.direction == Direction::Horizontal) {
        fixed_size.y = std::max(fixed_size.y, child->fixed_size.y);
        dynamic_size.y = std::max(dynamic_size.y, child->dynamic_size.y);
      } else {
        fixed_size.x = std::max(fixed_size.x, child->fixed_size.x);
        dynamic_size.x = std::max(dynamic_size.x, child->dynamic_size.x);
      }
    }

    if (auto width = std::get_if<LengthDynamic>(&style.width)) {
      if (style.direction == Direction::Horizontal) {
        dynamic_size.y = width->weight;
      } else {
        dynamic_size.x = width->weight;
      }
    }
  }

  fixed_size += style.outer_padding.size();
}

void SeriesElement::set_dependent_state(const std::vector<Element*>& children) {
  Vecf available = maximum(size - fixed_size, Vecf::Zero());
  float offset;
  if (style.direction == Direction::Horizontal) {
    offset = style.outer_padding.left - scroll_pos;
  } else {
    offset = style.outer_padding.top - scroll_pos;
  }

  // Node dynamic size may differ from sum of child dynamic sizes, so need to
  // re-calculate
  Vecf children_dynamic_size = Vecf::Zero();
  for (auto child : children) {
    children_dynamic_size += child->dynamic_size;
  }

  for (auto child : children) {
    if (child->hidden || child->zero_size()) {
      continue;
    }

    child->size = child->fixed_size;

    if (style.direction == Direction::Horizontal) {
      if (child->dynamic_size.x > 0) {
        child->size.x +=
            (child->dynamic_size.x / children_dynamic_size.x) * available.x;
      }
      if (child->dynamic_size.y > 0) {
        child->size.y = size.y - style.outer_padding.size().y;
      }

      child->position.x = position.x + offset;
      offset += child->size.x + style.inner_padding;

      switch (style.alignment) {
      case Alignment::Min:
        child->position.y = position.y + style.outer_padding.top;
        break;
      case Alignment::Center:
        child->position.y = position.y + size.y / 2 - child->size.y / 2;
        break;
      case Alignment::Max:
        child->position.y =
            position.y + size.y - child->size.y - style.outer_padding.bottom;
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
        child->size.x = size.x - style.outer_padding.size().x;
      }

      child->position.y = position.y + offset;
      offset += child->size.y + style.inner_padding;

      switch (style.alignment) {
      case Alignment::Min:
        child->position.x = position.x + style.outer_padding.left;
        break;
      case Alignment::Center:
        child->position.x = position.x + size.x / 2 - child->size.x / 2;
        break;
      case Alignment::Max:
        child->position.x =
            position.x + size.x - child->size.x - style.outer_padding.right;
        break;
      default:
        assert(false);
        break;
      }
    }
  }
}

void SeriesElement::render() const {
  resources->geometry_renderer
      .queue_box(box(), style.bg_color, 0, Color::Black(), 0);
}

bool SeriesElement::scroll_event(const ScrollEvent& event) {
  if (overrun == 0) {
    return false;
  }
  if (scroll_pos == overrun && event.amount > 0 ||
      scroll_pos == 0 && event.amount < 0) {
    return false;
  }

  scroll_pos = std::clamp(scroll_pos + event.amount, 0.f, overrun);
  return true;
}

} // namespace datagui
