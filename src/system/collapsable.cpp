#include "datagui/system/collapsable.hpp"
#include "datagui/system_utils/layout.hpp"

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
  state.fixed_size = collapsable.header_size;

  layout_set_input_state(
      element,
      theme,
      collapsable.layout,
      collapsable.layout_state);

  if (collapsable.fixed_size.x > 0) {
    state.fixed_size.x = collapsable.fixed_size.x;
  } else {
    state.fixed_size.x = std::max(
        collapsable.layout_state.content_fixed_size.x,
        collapsable.header_size.x);
  }

  state.fixed_size.y = collapsable.header_size.y;
  if (collapsable.open) {
    if (collapsable.fixed_size.y > 0) {
      state.fixed_size.y += collapsable.fixed_size.y;
    } else {
      state.fixed_size.y += collapsable.layout_state.content_fixed_size.y;
    }
  }

  if (collapsable.border) {
    state.fixed_size += Vec2::uniform(2 * theme->layout_border_width);
  }

  state.dynamic_size.x = collapsable.layout_state.content_dynamic_size.x;
  state.dynamic_size.y = 0;

  state.floating = false;
}

void CollapsableSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& collapsable = element.collapsable();

  for (auto child = element.child(); child; child = child.next()) {
    child.state().hidden = !collapsable.open;
  }
  if (!collapsable.open) {
    return;
  }

  collapsable.content_box = state.box();
  if (collapsable.border) {
    collapsable.content_box.lower += Vec2::uniform(theme->layout_border_width);
    collapsable.content_box.upper -= Vec2::uniform(theme->layout_border_width);
  }
  collapsable.content_box.lower.y += collapsable.header_size.y;

  layout_set_dependent_state(
      element,
      collapsable.content_box,
      theme,
      collapsable.layout,
      collapsable.layout_state);

  state.child_mask = collapsable.content_box;
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

  layout_render_scroll(
      collapsable.content_box,
      collapsable.layout_state,
      theme,
      renderer);
}

void CollapsableSystem::mouse_event(
    ElementPtr element,
    const MouseEvent& event) {
  const auto& state = element.state();
  auto& collapsable = element.collapsable();

  Box2 header_box(
      state.position,
      state.position + Vec2(state.size.x, collapsable.header_size.y));

  if (!header_box.contains(event.position)) {
    return;
  }

  if (event.action == MouseAction::Release &&
      event.button == MouseButton::Left) {
    collapsable.open = !collapsable.open;
  }
}

bool CollapsableSystem::scroll_event(
    ElementPtr element,
    const ScrollEvent& event) {
  auto& collapsable = element.collapsable();
  return layout_scroll_event(
      collapsable.content_box,
      collapsable.layout_state,
      event);
}

void CollapsableSystem::key_event(ElementPtr element, const KeyEvent& event) {
  auto& collapsable = element.collapsable();

  if (event.action == KeyAction::Release && event.key == Key::Enter) {
    collapsable.open = !collapsable.open;
  }
}

} // namespace datagui
