#include "datagui/system/split.hpp"

namespace datagui {

void SplitSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& split = element.split();

  state.fixed_size = Vec2();
  state.dynamic_size = Vec2();
  state.floating = false;

  Vec2 a_fixed_size;
  Vec2 a_dynamic_size;
  Vec2 b_fixed_size;
  Vec2 b_dynamic_size;

  auto first = element.child();
  if (first) {
    a_fixed_size = first.state().fixed_size;
    a_dynamic_size = first.state().dynamic_size;
    auto second = first.next();
    if (second) {
      b_fixed_size = first.state().fixed_size;
      b_dynamic_size = first.state().dynamic_size;
    }
  }

  if (split.direction == Direction::Horizontal) {
    state.fixed_size.x = std::max(a_fixed_size.x, b_fixed_size.x);
    state.fixed_size.y =
        a_fixed_size.y + b_fixed_size.y + theme->split_divider_width;
    state.dynamic_size.x = std::max(a_dynamic_size.x, b_dynamic_size.x);
    state.dynamic_size.y = 1;
  } else {
    state.fixed_size.x =
        a_fixed_size.x + b_fixed_size.x + theme->split_divider_width;
    state.fixed_size.y = std::max(a_fixed_size.y, b_fixed_size.y);
    state.dynamic_size.x = 1;
    state.dynamic_size.y = std::max(a_dynamic_size.y, b_dynamic_size.y);
  }
}

void SplitSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& split = element.split();

  state.child_mask = state.box();

  auto first = element.child();
  if (!first) {
    return;
  }

  float div_size = theme->split_divider_width;
  float line_offset =
      (theme->split_divider_width - theme->split_divider_line_width) / 2;

  first.state().position = element.state().position;
  if (split.direction == Direction::Horizontal) {
    float span = state.size.y;
    float half = split.ratio * span;
    float height_f = half - div_size / 2;
    float height_s = (span - half) - div_size / 2;

    auto& f_state = first.state();
    f_state.position = state.position;
    f_state.size = f_state.fixed_size;
    if (f_state.dynamic_size.x > 0) {
      f_state.size.x = state.size.x;
    }
    if (f_state.dynamic_size.y > 0) {
      f_state.size.y = height_f;
    }

    split.divider_box = state.box();
    split.divider_box.lower.y += (height_f + line_offset);
    split.divider_box.upper.y -= (height_s + line_offset);

    auto second = first.next();
    if (!second) {
      return;
    }
    auto& s_state = second.state();
    s_state.position = state.position;
    s_state.position.y += height_f + div_size;
    s_state.size = s_state.fixed_size;
    if (s_state.dynamic_size.x > 0) {
      s_state.size.x = state.size.x;
    }
    if (s_state.dynamic_size.y > 0) {
      s_state.size.y = height_s;
    }

  } else {
    float span = state.size.x;
    float half = split.ratio * span;
    float width_f = half - div_size / 2;
    float width_s = (span - half) - div_size / 2;

    auto& f_state = first.state();
    f_state.position = state.position;
    f_state.size = f_state.fixed_size;
    if (f_state.dynamic_size.x > 0) {
      f_state.size.x = width_f;
    }
    if (f_state.dynamic_size.y > 0) {
      f_state.size.y = state.size.y;
    }

    split.divider_box = state.box();
    split.divider_box.lower.x += (width_f + line_offset);
    split.divider_box.upper.x -= (width_s + line_offset);

    auto second = first.next();
    if (!second) {
      return;
    }
    auto& s_state = second.state();
    s_state.position = state.position;
    s_state.position.x += width_f + div_size;
    s_state.size = s_state.fixed_size;
    if (s_state.dynamic_size.x > 0) {
      s_state.size.x = width_s;
    }
    if (s_state.dynamic_size.y > 0) {
      s_state.size.y = state.size.y;
    }
  }
}

void SplitSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& split = element.split();
  renderer.queue_box(split.divider_box, theme->split_divider_color);
}

} // namespace datagui
