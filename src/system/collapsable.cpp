#include "datagui/system/collapsable.hpp"
#include "datagui/system_utils/layout.hpp"

namespace dgui {

void CollapsableSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& collapsable = element.collapsable();

  const auto& font =
      font_registry->get_font(theme->text_font, theme->text_size);
  collapsable.header_size = font.text_size(collapsable.label, LengthWrap()) +
                            Vec2::uniform(2 * theme->text_padding);
  state.fixed_size = collapsable.header_size;

  layout_set_input_state(
      element,
      theme,
      collapsable.layout,
      collapsable.layout_state);

  state.dynamic_size.x = collapsable.layout_state.content_dynamic_size.x;
  state.fixed_size.x = std::max(
      state.fixed_size.x,
      collapsable.layout_state.content_fixed_size.x);

  state.content_visible = collapsable.open;
  if (collapsable.open) {
    state.fixed_size.y += collapsable.layout_state.content_fixed_size.y;
    state.dynamic_size.y = collapsable.layout_state.content_dynamic_size.y;
  }

  if (collapsable.border) {
    state.fixed_size += Vec2::uniform(2 * theme->layout_border_width);
  }

  if (auto width = std::get_if<LengthFixed>(&collapsable.width)) {
    state.fixed_size.x = width->value;
    state.dynamic_size.x = 0;
  } else if (auto width = std::get_if<LengthDynamic>(&collapsable.width)) {
    state.dynamic_size.x = std::max(state.dynamic_size.x, width->weight);
  }
  if (auto height = std::get_if<LengthFixed>(&collapsable.height)) {
    state.fixed_size.y = height->value;
    state.dynamic_size.y = 0;
  } else if (auto height = std::get_if<LengthDynamic>(&collapsable.height)) {
    state.dynamic_size.y = std::max(state.dynamic_size.y, height->weight);
  }
}

void CollapsableSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& collapsable = element.collapsable();

  if (!collapsable.open) {
    for (auto child = element.child(); child; child = child.next()) {
      child.state().set_hidden();
    }
    return;
  }

  state.content_box = state.box();
  if (collapsable.border) {
    state.content_box.shrink(theme->layout_border_width);
  }
  state.content_box.lower.y += collapsable.header_size.y;

  layout_set_dependent_state(
      element,
      theme,
      collapsable.layout,
      collapsable.layout_state);

  state.content_overflowed =
      (collapsable.layout_state.content_overrun.x > 0 ||
       collapsable.layout_state.content_overrun.y > 0);
}

void CollapsableSystem::render(ConstElementPtr element, DrawList& dl) {
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
    dl.draw_box(state.box(), *collapsable.bg_color);
  }

  dl.draw_box(
      Box2(
          state.position,
          state.position + Vec2(state.size.x, collapsable.header_size.y)),
      header_color,
      border_width,
      theme->layout_border_color);

  Vec2 text_origin =
      state.position + Vec2::uniform(theme->text_padding + border_width);

  dl.draw_text(
      font_registry->get_font(theme->text_font, theme->text_size),
      text_origin,
      theme->text_color,
      LengthWrap(),
      collapsable.label);

  layout_render_scroll(state.content_box, collapsable.layout_state, theme, dl);
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
  const auto& state = element.state();
  auto& collapsable = element.collapsable();
  return layout_scroll_event(state.content_box, collapsable.layout_state, event);
}

void CollapsableSystem::key_event(ElementPtr element, const KeyEvent& event) {
  auto& collapsable = element.collapsable();
  if (event.action == KeyAction::Release && event.key == Key::Enter) {
    collapsable.open = !collapsable.open;
  }
}

} // namespace dgui
