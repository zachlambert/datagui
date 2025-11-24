#include "datagui/system/series.hpp"

namespace datagui {

void SeriesSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& series = element.series();

  state.fixed_size = Vecf::Zero();
  state.dynamic_size = Vecf::Zero();
  state.floating = false;

  float outer_padding = series.tight ? 0.f : theme->layout_outer_padding;
  float inner_padding = series.tight ? 0.f : theme->layout_inner_padding;
  if (series.border) {
    outer_padding += theme->layout_border_width;
  }

  // Primary direction

  {
    std::size_t child_count = 0;
    for (auto child = element.child(); child; child = child.next()) {
      const auto& c_state = child.state();
      if (c_state.hidden || c_state.zero_size()) {
        continue;
      }

      if (series.direction == Direction::Horizontal) {
        state.fixed_size.x += c_state.fixed_size.x;
        state.dynamic_size.x += c_state.dynamic_size.x;
      } else {
        state.fixed_size.y += c_state.fixed_size.y;
        state.dynamic_size.y += c_state.dynamic_size.y;
      }
      child_count++;
    }

    if (child_count > 0) {
      if (series.direction == Direction::Horizontal) {
        state.fixed_size.x += (child_count - 1) * inner_padding;
      } else {
        state.fixed_size.y += (child_count - 1) * inner_padding;
      }
    }

    if (series.direction == Direction::Horizontal) {
      series.content_length = state.fixed_size.x;
    } else {
      series.content_length = state.fixed_size.y;
    }
  }
  if (auto length = std::get_if<LengthFixed>(&series.length)) {
    if (series.direction == Direction::Horizontal) {
      state.fixed_size.x = length->value;
    } else {
      state.fixed_size.y = length->value;
    }
  } else if (auto length = std::get_if<LengthDynamic>(&series.length)) {
    if (series.direction == Direction::Horizontal) {
      state.dynamic_size.y = length->weight;
    } else {
      state.dynamic_size.x = length->weight;
    }
  }

  // Secondary direction

  if (auto width = std::get_if<LengthFixed>(&series.width)) {
    if (series.direction == Direction::Horizontal) {
      state.fixed_size.y = width->value;
    } else {
      state.fixed_size.x = width->value;
    }
  } else {
    for (auto child = element.child(); child; child = child.next()) {
      const auto& c_state = child.state();
      if (c_state.hidden || c_state.zero_size()) {
        continue;
      }

      if (series.direction == Direction::Horizontal) {
        state.fixed_size.y = std::max(state.fixed_size.y, c_state.fixed_size.y);
        state.dynamic_size.y =
            std::max(state.dynamic_size.y, c_state.dynamic_size.y);
      } else {
        state.fixed_size.x = std::max(state.fixed_size.x, c_state.fixed_size.x);
        state.dynamic_size.x =
            std::max(state.dynamic_size.x, c_state.dynamic_size.x);
      }
    }

    if (auto width = std::get_if<LengthDynamic>(&series.width)) {
      if (series.direction == Direction::Horizontal) {
        state.dynamic_size.y = width->weight;
      } else {
        state.dynamic_size.x = width->weight;
      }
    }
  }

  state.fixed_size += Vecf::Constant(2 * outer_padding);
  series.content_length += 2 * outer_padding;
}

void SeriesSystem::set_dependent_state(ElementPtr element) {
  const auto& state = element.state();
  auto& series = element.series();

  float outer_padding = series.tight ? 0.f : theme->layout_outer_padding;
  float inner_padding = series.tight ? 0.f : theme->layout_inner_padding;
  if (series.border) {
    outer_padding += theme->layout_border_width;
  }

  Vecf available = maximum(state.size - state.fixed_size, Vecf::Zero());
  float offset = outer_padding - series.scroll_pos;

  if (series.direction == Direction::Horizontal) {
    series.overrun = std::max(series.content_length - state.size.x, 0.f);
  } else {
    series.overrun = std::max(series.content_length - state.size.y, 0.f);
  }

  // Node dynamic size may differ from sum of child dynamic sizes, so need to
  // re-calculate
  Vecf children_dynamic_size = Vecf::Zero();
  for (auto child = element.child(); child; child = child.next()) {
    children_dynamic_size += child.state().dynamic_size;
  }

  for (auto child = element.child(); child; child = child.next()) {
    auto& c_state = child.state();
    if (c_state.hidden || c_state.zero_size()) {
      continue;
    }

    c_state.size = c_state.fixed_size;

    if (series.direction == Direction::Horizontal) {
      if (c_state.dynamic_size.x > 0) {
        c_state.size.x +=
            (c_state.dynamic_size.x / children_dynamic_size.x) * available.x;
      }
      if (c_state.dynamic_size.y > 0) {
        c_state.size.y = state.size.y - 2.f * outer_padding;
      }

      c_state.position.x = state.position.x + offset;
      offset += c_state.size.x + inner_padding;

      switch (series.alignment) {
      case Alignment::Min:
        c_state.position.y = state.position.y + outer_padding;
        break;
      case Alignment::Center:
        c_state.position.y =
            state.position.y + state.size.y / 2 - c_state.size.y / 2;
        break;
      case Alignment::Max:
        c_state.position.y =
            state.position.y + state.size.y - c_state.size.y - outer_padding;
        break;
      default:
        assert(false);
        break;
      }

    } else {
      if (c_state.dynamic_size.y > 0) {
        c_state.size.y +=
            (c_state.dynamic_size.y / children_dynamic_size.y) * available.y;
      }
      if (c_state.dynamic_size.x > 0) {
        c_state.size.x = state.size.x - 2.f * outer_padding;
      }

      c_state.position.y = state.position.y + offset;
      offset += c_state.size.y + inner_padding;

      switch (series.alignment) {
      case Alignment::Min:
        c_state.position.x = state.position.x + outer_padding;
        break;
      case Alignment::Center:
        c_state.position.x =
            state.position.x + state.size.x / 2 - c_state.size.x / 2;
        break;
      case Alignment::Max:
        c_state.position.x =
            state.position.x + state.size.x - c_state.size.x - outer_padding;
        break;
      default:
        assert(false);
        break;
      }
    }
  }
}

void SeriesSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& series = element.series();

  if (series.bg_color || series.border) {
    Color bg_color = series.bg_color ? *series.bg_color : Color::Clear();
    int border_width =
        series.border ? theme->layout_border_width : theme->layout_border_width;
    renderer.queue_box(
        state.box(),
        bg_color,
        border_width,
        theme->layout_border_color,
        0);
  }

  if (series.overrun > 0) {
    Vecf origin = state.position;
    Vecf size = state.size;
    if (series.border) {
      origin += Vecf::Constant(theme->layout_border_width);
      size -= 2.f * Vecf::Constant(theme->layout_border_width);
    }
    Boxf bg;
    Boxf fg;
    if (series.direction == Direction::Vertical) {
      bg.lower.x = origin.x + size.x - theme->scroll_bar_width;
      bg.upper.x = origin.x + size.x;
      bg.lower.y = origin.y;
      bg.upper.y = origin.y + size.y;

      float ratio = size.y / (series.overrun + size.y);
      float location = series.scroll_pos / (series.overrun + size.y);

      fg = bg;
      fg.lower.y = origin.y + location * size.y;
      fg.upper.y = origin.y + (location + ratio) * size.y;

    } else {
      bg.lower.x = origin.x;
      bg.upper.x = origin.x + size.x;
      bg.lower.y = origin.y + size.y - theme->scroll_bar_width;
      bg.upper.y = origin.y + size.y;

      float ratio = size.x / (series.overrun + size.x);
      float location = series.scroll_pos / (series.overrun + size.x);

      fg = bg;
      fg.lower.x = origin.x + location * size.x;
      fg.upper.x = origin.x + (location + ratio) * size.x;
    }
    renderer.queue_box(bg, theme->scroll_bar_bg, 0, Color::Black(), 0);
    renderer.queue_box(fg, theme->scroll_bar_fg, 0, Color::Black(), 0);
  }
}

bool SeriesSystem::scroll_event(ElementPtr element, const ScrollEvent& event) {
  auto& series = element.series();

  if (series.overrun == 0) {
    return false;
  }
  if (series.scroll_pos == series.overrun && event.amount > 0 ||
      series.scroll_pos == 0 && event.amount < 0) {
    return false;
  }

  series.scroll_pos =
      std::clamp(series.scroll_pos + event.amount, 0.f, series.overrun);
  return true;
}

} // namespace datagui
