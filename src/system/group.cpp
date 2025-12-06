#include "datagui/system/group.hpp"

namespace datagui {

void GroupSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& group = element.group();

  state.fixed_size = Vec2();
  state.dynamic_size = Vec2();
  state.floating = false;

  float outer_padding = group.tight ? 0.f : theme->layout_outer_padding;
  float inner_padding = group.tight ? 0.f : theme->layout_inner_padding;
  if (group.border) {
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

      if (group.direction == Direction::Horizontal) {
        state.fixed_size.x += c_state.fixed_size.x;
        state.dynamic_size.x += c_state.dynamic_size.x;
      } else {
        state.fixed_size.y += c_state.fixed_size.y;
        state.dynamic_size.y += c_state.dynamic_size.y;
      }
      child_count++;
    }

    if (child_count > 0) {
      if (group.direction == Direction::Horizontal) {
        state.fixed_size.x += (child_count - 1) * inner_padding;
      } else {
        state.fixed_size.y += (child_count - 1) * inner_padding;
      }
    }

    if (group.direction == Direction::Horizontal) {
      group.content_length = state.fixed_size.x;
    } else {
      group.content_length = state.fixed_size.y;
    }
  }
  if (auto length = std::get_if<LengthFixed>(&group.length)) {
    if (group.direction == Direction::Horizontal) {
      state.fixed_size.x = length->value;
    } else {
      state.fixed_size.y = length->value;
    }
  } else if (auto length = std::get_if<LengthDynamic>(&group.length)) {
    if (group.direction == Direction::Horizontal) {
      state.dynamic_size.y = length->weight;
    } else {
      state.dynamic_size.x = length->weight;
    }
  }

  // Secondary direction

  if (auto width = std::get_if<LengthFixed>(&group.width)) {
    if (group.direction == Direction::Horizontal) {
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

      if (group.direction == Direction::Horizontal) {
        state.fixed_size.y = std::max(state.fixed_size.y, c_state.fixed_size.y);
        state.dynamic_size.y =
            std::max(state.dynamic_size.y, c_state.dynamic_size.y);
      } else {
        state.fixed_size.x = std::max(state.fixed_size.x, c_state.fixed_size.x);
        state.dynamic_size.x =
            std::max(state.dynamic_size.x, c_state.dynamic_size.x);
      }
    }

    if (auto width = std::get_if<LengthDynamic>(&group.width)) {
      if (group.direction == Direction::Horizontal) {
        state.dynamic_size.y = width->weight;
      } else {
        state.dynamic_size.x = width->weight;
      }
    }
  }

  state.fixed_size += Vec2::uniform(2 * outer_padding);
  group.content_length += 2 * outer_padding;
}

void GroupSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& group = element.group();

  state.child_mask = state.box();
  if (group.border) {
    state.child_mask.lower += Vec2::uniform(theme->layout_border_width);
    state.child_mask.upper -= Vec2::uniform(theme->layout_border_width);
  }

  float outer_padding = group.tight ? 0.f : theme->layout_outer_padding;
  float inner_padding = group.tight ? 0.f : theme->layout_inner_padding;
  if (group.border) {
    outer_padding += theme->layout_border_width;
  }

  Vec2 available = maximum(state.size - state.fixed_size, Vec2());
  float offset = outer_padding - group.scroll_pos;

  if (group.direction == Direction::Horizontal) {
    group.overrun = std::max(group.content_length - state.size.x, 0.f);
  } else {
    group.overrun = std::max(group.content_length - state.size.y, 0.f);
  }

  // Node dynamic size may differ from sum of child dynamic sizes, so need to
  // re-calculate
  Vec2 children_dynamic_size = Vec2();
  for (auto child = element.child(); child; child = child.next()) {
    children_dynamic_size += child.state().dynamic_size;
  }

  for (auto child = element.child(); child; child = child.next()) {
    auto& c_state = child.state();
    if (c_state.hidden || c_state.zero_size()) {
      continue;
    }

    c_state.size = c_state.fixed_size;

    if (group.direction == Direction::Horizontal) {
      if (c_state.dynamic_size.x > 0) {
        c_state.size.x +=
            (c_state.dynamic_size.x / children_dynamic_size.x) * available.x;
      }
      if (c_state.dynamic_size.y > 0) {
        c_state.size.y = state.size.y - 2.f * outer_padding;
      }

      c_state.position.x = state.position.x + offset;
      offset += c_state.size.x + inner_padding;

      switch (group.alignment) {
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

      switch (group.alignment) {
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

void GroupSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& group = element.group();

  if (group.bg_color || group.border) {
    Color bg_color = group.bg_color ? *group.bg_color : Color::Clear();
    int border_width =
        group.border ? theme->layout_border_width : theme->layout_border_width;
    renderer.queue_box(
        state.box(),
        bg_color,
        border_width,
        theme->layout_border_color);
  }

  if (group.overrun > 0) {
    Vec2 origin = state.position;
    Vec2 size = state.size;
    if (group.border) {
      origin += Vec2::uniform(theme->layout_border_width);
      size -= 2.f * Vec2::uniform(theme->layout_border_width);
    }
    Box2 bg;
    Box2 fg;
    if (group.direction == Direction::Vertical) {
      bg.lower.x = origin.x + size.x - theme->scroll_bar_width;
      bg.upper.x = origin.x + size.x;
      bg.lower.y = origin.y;
      bg.upper.y = origin.y + size.y;

      float ratio = size.y / (group.overrun + size.y);
      float location = group.scroll_pos / (group.overrun + size.y);

      fg = bg;
      fg.lower.y = origin.y + location * size.y;
      fg.upper.y = origin.y + (location + ratio) * size.y;

    } else {
      bg.lower.x = origin.x;
      bg.upper.x = origin.x + size.x;
      bg.lower.y = origin.y + size.y - theme->scroll_bar_width;
      bg.upper.y = origin.y + size.y;

      float ratio = size.x / (group.overrun + size.x);
      float location = group.scroll_pos / (group.overrun + size.x);

      fg = bg;
      fg.lower.x = origin.x + location * size.x;
      fg.upper.x = origin.x + (location + ratio) * size.x;
    }
    renderer.queue_box(bg, theme->scroll_bar_bg);
    renderer.queue_box(fg, theme->scroll_bar_fg);
  }
}

bool GroupSystem::scroll_event(ElementPtr element, const ScrollEvent& event) {
  auto& group = element.group();

  if (group.overrun == 0) {
    return false;
  }
  if (group.scroll_pos == group.overrun && event.amount > 0 ||
      group.scroll_pos == 0 && event.amount < 0) {
    return false;
  }

  group.scroll_pos =
      std::clamp(group.scroll_pos + event.amount, 0.f, group.overrun);
  return true;
}

} // namespace datagui
