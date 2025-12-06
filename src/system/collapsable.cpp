#include "datagui/system/collapsable.hpp"

namespace datagui {

void CollapsableSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& collapsable = element.collapsable();

  collapsable.header_size = fm->text_size(
                                collapsable.label,
                                theme->text_font,
                                theme->text_size,
                                LengthWrap()) +
                            Vec2::uniform(2 * theme->text_padding);
  if (collapsable.border) {
    collapsable.header_size += Vec2::uniform(2 * theme->layout_border_width);
  }
  state.fixed_size = collapsable.header_size;

  state.dynamic_size.x = 1;
  state.dynamic_size.y = 0;
  state.floating = false;

  auto child = element.child();
  if (!child) {
    return;
  }

  Vec2 content_size = child.state().fixed_size;
  if (!collapsable.tight) {
    content_size += 2.f * Vec2::uniform(theme->layout_outer_padding);
  }

  state.fixed_size.x = std::max(state.fixed_size.x, content_size.x);
  if (collapsable.open) {
    state.fixed_size.y += content_size.y;
  }
}

void CollapsableSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  const auto& collapsable = element.collapsable();

  auto child = element.child();
  if (!child) {
    state.child_mask = state.box();
    return;
  }
  for (auto other = child.next(); other; other = other.next()) {
    other.state().hidden = true;
  }

  if (!collapsable.open) {
    child.state().hidden = true;
    return;
  }
  child.state().hidden = false;

  child.state().position = state.position + Vec2(0, collapsable.header_size.y);
  if (!collapsable.tight) {
    child.state().position += Vec2::uniform(theme->layout_outer_padding);
  }

  Vec2 available_size = state.size;
  available_size.y -= collapsable.header_size.y;
  if (!collapsable.tight) {
    available_size -= Vec2::uniform(2 * theme->layout_outer_padding);
  }

  if (child.state().dynamic_size.x > 0) {
    child.state().size.x = available_size.x;
  } else {
    child.state().size.x = child.state().fixed_size.x;
  }
  if (child.state().dynamic_size.y > 0) {
    child.state().size.y = available_size.y;
  } else {
    child.state().size.y = child.state().fixed_size.y;
  }

  state.child_mask = child.state().box();
}

void CollapsableSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& collapsable = element.collapsable();

  Color header_color = collapsable.header_color ? *collapsable.header_color
                                                : theme->layout_color_bg;
  int border_width = collapsable.border ? theme->layout_border_width : 0;
  if (collapsable.open) {
    header_color.r += 0.5 * (1 - header_color.r);
    header_color.g += 0.5 * (1 - header_color.g);
    header_color.b += 0.5 * (1 - header_color.b);
  }

  if (collapsable.bg_color) {
    renderer.queue_box(state.box(), *collapsable.bg_color);
  }

  renderer.queue_box(
      Box2(
          state.position,
          state.position + Vec2(state.size.x, collapsable.header_size.y)),
      header_color,
      border_width,
      theme->layout_border_color);

  Vec2 text_origin =
      state.position + Vec2::uniform(theme->text_padding + border_width);
  renderer.queue_text(
      text_origin,
      collapsable.label,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthWrap());
}

bool CollapsableSystem::mouse_event(
    ElementPtr element,
    const MouseEvent& event) {
  const auto& state = element.state();
  auto& collapsable = element.collapsable();

  Box2 header_box(
      state.position,
      state.position + Vec2(state.size.x, collapsable.header_size.y));

  if (!header_box.contains(event.position)) {
    return false;
  }

  if (event.action == MouseAction::Release &&
      event.button == MouseButton::Left) {
    collapsable.open = !collapsable.open;
  }
  return false;
}

bool CollapsableSystem::key_event(ElementPtr element, const KeyEvent& event) {
  auto& collapsable = element.collapsable();

  if (event.action == KeyAction::Release && event.key == Key::Enter) {
    collapsable.open = !collapsable.open;
  }
  return false;
}

} // namespace datagui
