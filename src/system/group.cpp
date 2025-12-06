#include "datagui/system/group.hpp"
#include "datagui/system_utils/layout.hpp"

namespace datagui {

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

  if (group.fixed_size.x > 0) {
    state.fixed_size.x = group.fixed_size.x;
    state.dynamic_size.x = 0;
  }
  if (group.fixed_size.y > 0) {
    state.fixed_size.y = group.fixed_size.y;
    state.dynamic_size.y = 0;
  }
}

void GroupSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& group = element.group();

  group.content_box = state.box();
  if (group.border) {
    group.content_box.lower += Vec2::uniform(theme->layout_border_width);
    group.content_box.upper -= Vec2::uniform(theme->layout_border_width);
  }
  state.child_mask = group.content_box;

  layout_set_dependent_state(
      element,
      group.content_box,
      theme,
      group.layout,
      group.layout_state);
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

  layout_render_scroll(group.content_box, group.layout_state, theme, renderer);
}

bool GroupSystem::scroll_event(ElementPtr element, const ScrollEvent& event) {
  auto& group = element.group();
  return layout_scroll_event(group.content_box, group.layout_state, event);
}

} // namespace datagui
