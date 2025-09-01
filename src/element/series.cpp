#include "datagui/element/series.hpp"

namespace datagui {

void SeriesSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  auto& props = *e.props.cast<SeriesProps>();

  e.fixed_size = Vecf::Zero();
  e.dynamic_size = Vecf::Zero();
  e.floating = false;

  // Primary direction

  {
    for (auto child : children) {
      if (child->hidden || child->zero_size()) {
        continue;
      }

      if (props.direction == Direction::Horizontal) {
        e.fixed_size.x += child->fixed_size.x;
        e.dynamic_size.x += child->dynamic_size.x;
      } else {
        e.fixed_size.y += child->fixed_size.y;
        e.dynamic_size.y += child->dynamic_size.y;
      }
    }

    if (!children.empty()) {
      if (props.direction == Direction::Horizontal) {
        e.fixed_size.x += (children.size() - 1) * theme->layout_inner_padding;
      } else {
        e.fixed_size.y += (children.size() - 1) * theme->layout_inner_padding;
      }
    }
  }
  if (auto length = std::get_if<LengthFixed>(&props.length)) {
    if (props.direction == Direction::Horizontal) {
      props.overrun = std::max(e.fixed_size.x - length->value, 0.f);
      e.fixed_size.x = length->value;
    } else {
      props.overrun = std::max(e.fixed_size.y - length->value, 0.f);
      e.fixed_size.y = length->value;
    }
  } else if (auto length = std::get_if<LengthDynamic>(&props.length)) {
    if (props.direction == Direction::Horizontal) {
      e.dynamic_size.y = length->weight;
    } else {
      e.dynamic_size.x = length->weight;
    }
  }

  // Secondary direction

  if (auto width = std::get_if<LengthFixed>(&props.width)) {
    if (props.direction == Direction::Horizontal) {
      e.fixed_size.y = width->value;
    } else {
      e.fixed_size.x = width->value;
    }
  } else {
    for (auto child : children) {
      if (child->hidden || child->zero_size()) {
        continue;
      }

      if (props.direction == Direction::Horizontal) {
        e.fixed_size.y = std::max(e.fixed_size.y, child->fixed_size.y);
        e.dynamic_size.y = std::max(e.dynamic_size.y, child->dynamic_size.y);
      } else {
        e.fixed_size.x = std::max(e.fixed_size.x, child->fixed_size.x);
        e.dynamic_size.x = std::max(e.dynamic_size.x, child->dynamic_size.x);
      }
    }

    if (auto width = std::get_if<LengthDynamic>(&props.width)) {
      if (props.direction == Direction::Horizontal) {
        e.dynamic_size.y = width->weight;
      } else {
        e.dynamic_size.x = width->weight;
      }
    }
  }

  e.fixed_size += Vecf::Constant(theme->layout_outer_padding);
}

void SeriesSystem::set_dependent_state(
    Element& e,
    const ElementList& children) {
  const auto& props = *e.props.cast<SeriesProps>();

  Vecf available = maximum(e.size - e.fixed_size, Vecf::Zero());
  float offset;
  if (props.direction == Direction::Horizontal) {
    offset = theme->layout_outer_padding - props.scroll_pos;
  } else {
    offset = theme->layout_outer_padding - props.scroll_pos;
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

    if (props.direction == Direction::Horizontal) {
      if (child->dynamic_size.x > 0) {
        child->size.x +=
            (child->dynamic_size.x / children_dynamic_size.x) * available.x;
      }
      if (child->dynamic_size.y > 0) {
        child->size.y = e.size.y - 2.f * theme->layout_outer_padding;
      }

      child->position.x = e.position.x + offset;
      offset += child->size.x + theme->layout_inner_padding;

      switch (props.alignment) {
      case Alignment::Min:
        child->position.y = e.position.y + theme->layout_outer_padding;
        break;
      case Alignment::Center:
        child->position.y = e.position.y + e.size.y / 2 - child->size.y / 2;
        break;
      case Alignment::Max:
        child->position.y = e.position.y + e.size.y - child->size.y -
                            theme->layout_outer_padding;
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
        child->size.x = e.size.x - 2.f * theme->layout_outer_padding;
      }

      child->position.y = e.position.y + offset;
      offset += child->size.y + theme->layout_inner_padding;

      switch (props.alignment) {
      case Alignment::Min:
        child->position.x = e.position.x + theme->layout_outer_padding;
        break;
      case Alignment::Center:
        child->position.x = e.position.x + e.size.x / 2 - child->size.x / 2;
        break;
      case Alignment::Max:
        child->position.x = e.position.x + e.size.x - child->size.x -
                            theme->layout_outer_padding;
        break;
      default:
        assert(false);
        break;
      }
    }
  }
}

void SeriesSystem::render(const Element& e, Renderer& renderer) {
  const auto& props = *e.props.cast<SeriesProps>();
  renderer.queue_box(e.box(), theme->layout_color_bg, 0, Color::Black(), 0);
}

bool SeriesSystem::scroll_event(Element& e, const ScrollEvent& event) {
  auto& props = *e.props.cast<SeriesProps>();
  if (props.overrun == 0) {
    return false;
  }
  if (props.scroll_pos == props.overrun && event.amount > 0 ||
      props.scroll_pos == 0 && event.amount < 0) {
    return false;
  }

  props.scroll_pos =
      std::clamp(props.scroll_pos + event.amount, 0.f, props.overrun);
  return true;
}

} // namespace datagui
