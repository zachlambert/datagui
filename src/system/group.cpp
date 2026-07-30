#include "datagui/system/group.hpp"
#include "datagui/system_utils/layout.hpp"

namespace dgui {

void GroupSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& group = element.group();

  layout_set_input_state(element, theme, group.layout, group.layout_state);

  state.fixed_size = group.layout_state.content_fixed_size;
  state.dynamic_size = group.layout_state.content_dynamic_size;
  state.floating = false;
  if (group.border) {
    state.fixed_size += Vec2::uniform(2 * theme->layout_border_width);
  }

  if (auto width = std::get_if<LengthFixed>(&group.width)) {
    state.fixed_size.x = width->value;
    state.dynamic_size.x = 0;
  } else if (auto width = std::get_if<LengthDynamic>(&group.width)) {
    state.dynamic_size.x = std::max(state.dynamic_size.x, width->weight);
  }
  if (auto height = std::get_if<LengthFixed>(&group.height)) {
    state.fixed_size.y = height->value;
    state.dynamic_size.y = 0;
  } else if (auto height = std::get_if<LengthDynamic>(&group.height)) {
    state.dynamic_size.y = std::max(state.dynamic_size.y, height->weight);
  }

  state.floating = false;
}

void GroupSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& group = element.group();

  group.layout_state.content_box = state.box();
  if (group.border) {
    group.layout_state.content_box.lower +=
        Vec2::uniform(theme->layout_border_width);
    group.layout_state.content_box.upper -=
        Vec2::uniform(theme->layout_border_width);
  }

  layout_set_dependent_state(element, theme, group.layout, group.layout_state);
}

void GroupSystem::render(ConstElementPtr element, DrawList& dl) {
  const auto& state = element.state();
  const auto& group = element.group();

  if (group.bg_color || group.border) {
    Color bg_color = group.bg_color ? *group.bg_color : Color::Clear();
    int border_width =
        group.border ? theme->layout_border_width : theme->layout_border_width;
    dl.draw_box(
        state.box(),
        bg_color,
        border_width,
        theme->layout_border_color);
  }

  layout_render(group.layout_state, theme, dl);
}

bool GroupSystem::scroll_event(ElementPtr element, const ScrollEvent& event) {
  auto& group = element.group();
  return layout_scroll_event(group.layout_state, event);
}

} // namespace dgui
