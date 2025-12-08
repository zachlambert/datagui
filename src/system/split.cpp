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

  ElementPtr first, second;
  for (auto child = element.child(); child; child = child.next()) {
    if (child.state().float_only) {
      continue;
    }
    if (!first) {
      first = child;
    } else {
      second = child;
      break;
    }
  }

  if (first) {
    a_fixed_size = first.state().fixed_size;
    a_dynamic_size = first.state().dynamic_size;
    auto second = first.next();
    if (second) {
      b_fixed_size = second.state().fixed_size;
      b_dynamic_size = second.state().dynamic_size;
    }
  }

  if (split.direction == Direction::Horizontal) {
    state.fixed_size.y =
        a_fixed_size.y + b_fixed_size.y + theme->split_divider_width;
    state.dynamic_size.y = std::max(a_dynamic_size.y + b_dynamic_size.y, 1.f);

    state.fixed_size.x = std::max(a_fixed_size.x, b_fixed_size.x);
    state.dynamic_size.x = std::max(a_dynamic_size.x, b_dynamic_size.x);
  } else {
    state.fixed_size.x =
        a_fixed_size.x + b_fixed_size.x + theme->split_divider_width;
    state.dynamic_size.x = std::max(a_dynamic_size.x + b_dynamic_size.x, 1.f);

    state.fixed_size.y = std::max(a_fixed_size.y, b_fixed_size.y);
    state.dynamic_size.y = std::max(a_dynamic_size.y, b_dynamic_size.y);
  }

  if (auto width = std::get_if<LengthFixed>(&split.width)) {
    state.fixed_size.x = width->value;
    state.dynamic_size.x = 0;
  } else if (auto width = std::get_if<LengthDynamic>(&split.width)) {
    state.dynamic_size.x = std::max(state.dynamic_size.x, width->weight);
  }
  if (auto height = std::get_if<LengthFixed>(&split.height)) {
    state.fixed_size.y = height->value;
    state.dynamic_size.y = 0;
  } else if (auto height = std::get_if<LengthDynamic>(&split.height)) {
    state.dynamic_size.y = std::max(state.dynamic_size.y, height->weight);
  }
}

void SplitSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& split = element.split();

  state.child_mask = state.box();

  auto first = element.child();
  while (first && first.state().float_only) {
    first = first.next();
  }
  if (!first) {
    return;
  }

  auto second = first.next();
  while (second && second.state().float_only) {
    second = second.next();
  }

  float div_size = theme->split_divider_width;

  first.state().position = element.state().position;
  if (split.direction == Direction::Horizontal) {
    float span = state.size.y;
    float half = split.ratio * span;
    float height_f = half - div_size / 2;
    float height_s = (span - half) - div_size / 2;

    auto& f_state = first.state();
    f_state.position = state.position;
    f_state.size = minimum(f_state.fixed_size, Vec2(state.size.x, height_f));
    if (f_state.dynamic_size.x > 0) {
      f_state.size.x = state.size.x;
    }
    if (f_state.dynamic_size.y > 0) {
      f_state.size.y = height_f;
    }

    split.divider_box = state.box();
    split.divider_box.lower.y += height_f;
    split.divider_box.upper.y -= height_s;

    auto second = first.next();
    if (!second) {
      return;
    }
    auto& s_state = second.state();
    s_state.position = state.position;
    s_state.position.y += height_f + div_size;
    s_state.size = minimum(s_state.fixed_size, Vec2(state.size.x, height_s));
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
    f_state.size = minimum(f_state.fixed_size, Vec2(width_f, state.size.y));
    if (f_state.dynamic_size.x > 0) {
      f_state.size.x = width_f;
    }
    if (f_state.dynamic_size.y > 0) {
      f_state.size.y = state.size.y;
    }

    split.divider_box = state.box();
    split.divider_box.lower.x += width_f;
    split.divider_box.upper.x -= width_s;

    auto second = first.next();
    if (!second) {
      return;
    }
    auto& s_state = second.state();
    s_state.position = state.position;
    s_state.position.x += width_f + div_size;
    s_state.size = minimum(s_state.fixed_size, Vec2(width_s, state.size.y));
    if (s_state.dynamic_size.x > 0) {
      s_state.size.x = width_s;
    }
    if (s_state.dynamic_size.y > 0) {
      s_state.size.y = state.size.y;
    }
  }
}

void SplitSystem::render(ConstElementPtr element, GuiRenderer& renderer) {
  const auto& split = element.split();
  const Color& color = split.held ? theme->split_divider_color_active
                                  : theme->split_divider_color;
  renderer.queue_box(split.divider_box, color);
}

void SplitSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& split = element.split();

  if (event.action == MouseAction::Release) {
    split.held = false;
  }
  if (event.action == MouseAction::Press) {
    if (split.divider_box.contains(event.position)) {
      // Allow held=true for fixed split too, to visually show it is held,
      // but don't update the ratio
      split.held = true;
    }
    return;
  }
  if (!split.held || split.fixed) {
    return;
  }

  if (split.direction == Direction::Horizontal) {
    split.ratio = std::clamp(
        (event.position.y - state.position.y) / state.size.y,
        0.f,
        1.f);
  } else {
    split.ratio = std::clamp(
        (event.position.x - state.position.x) / state.size.x,
        0.f,
        1.f);
  }
}

} // namespace datagui
