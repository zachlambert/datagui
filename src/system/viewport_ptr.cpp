#include "datagui/system/viewport_ptr.hpp"
#include "datagui/system_utils/layout.hpp"

namespace dgui {

void ViewportPtrSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& viewport = element.viewport();

  // NOTE: Copy of group layout logic

  layout_set_input_state(
      element,
      theme,
      viewport.layout,
      viewport.layout_state);

  state.fixed_size = viewport.layout_state.content_fixed_size;
  state.dynamic_size = viewport.layout_state.content_dynamic_size;
  if (viewport.border) {
    state.fixed_size += Vec2::uniform(2 * theme->layout_border_width);
  }

  if (auto width = std::get_if<LengthFixed>(&viewport.width)) {
    state.fixed_size.x = width->value;
    state.dynamic_size.x = 0;
  } else if (auto width = std::get_if<LengthDynamic>(&viewport.width)) {
    state.dynamic_size.x = std::max(state.dynamic_size.x, width->weight);
  }
  if (auto height = std::get_if<LengthFixed>(&viewport.height)) {
    state.fixed_size.y = height->value;
    state.dynamic_size.y = 0;
  } else if (auto height = std::get_if<LengthDynamic>(&viewport.height)) {
    state.dynamic_size.y = std::max(state.dynamic_size.y, height->weight);
  }

  // NOTE: Currently setting this to true so it always forces a new group
  // for drawing the content (scene)
  // The executor will draw scenes before any geometry/text/image
  state.content_overflowed = true;
}

void ViewportPtrSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& viewport = element.viewport();

  state.content_box = state.box();
  if (viewport.border) {
    state.content_box.shrink(theme->layout_border_width);
  }

  layout_set_dependent_state(
      element,
      theme,
      viewport.layout,
      viewport.layout_state);
}

void ViewportPtrSystem::render(ConstElementPtr element, DrawList& dl) {
  const auto& state = element.state();
  auto& viewport = element.viewport();
  if (viewport.border) {
    dl.draw_box(
        state.box(),
        Color::Clear(),
        theme->layout_border_width,
        theme->layout_border_color);
  }
}

void ViewportPtrSystem::render_content(ConstElementPtr element, DrawList& dl) {
  const auto& state = element.state();
  auto& viewport = element.viewport();
  viewport.viewport->draw(state.content_box, dl);
}

void ViewportPtrSystem::mouse_event(
    ElementPtr element,
    const MouseEvent& event) {
  const auto& state = element.state();
  const auto& viewport = element.viewport();

  MouseEvent remapped = event;
  remapped.position = state.box().to_coords(event.position);
  remapped.press_position = state.box().to_coords(event.press_position);
  remapped.position.y = 1 - remapped.position.y;
  remapped.press_position.y = 1 - remapped.press_position.y;
  viewport.viewport->mouse_event(remapped);
}

bool ViewportPtrSystem::scroll_event(
    ElementPtr element,
    const ScrollEvent& event) {
  const auto& state = element.state();
  const auto& viewport = element.viewport();

  ScrollEvent remapped = event;
  remapped.position = state.box().to_coords(event.position);
  remapped.position.y = 1 - remapped.position.y;
  return viewport.viewport->scroll_event(remapped);
}

} // namespace dgui
