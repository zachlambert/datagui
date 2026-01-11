#include "datagui/system/viewport_ptr.hpp"

namespace datagui {

void ViewportPtrSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& viewport = element.viewport();

  state.fixed_size = Vec2(viewport.width, viewport.height);
  state.dynamic_size = Vec2();
  state.floating = false;
}

void ViewportPtrSystem::render(ConstElementPtr element, GuiRenderer& renderer) {
  const auto& state = element.state();
  const auto& viewport = element.viewport();
  renderer.queue_viewport(
      Box2(state.position, state.position + state.fixed_size),
      viewport.viewport->texture());
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

} // namespace datagui
