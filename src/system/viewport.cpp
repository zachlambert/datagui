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
  renderer.queue_texture(state.box(), viewport.viewport->texture());
}

void ViewportPtrSystem::mouse_event(
    ElementPtr element,
    const MouseEvent& event) {
  const auto& state = element.state();
  const auto& viewport = element.viewport();

  MouseEvent event_viewport = event;
  event_viewport.position.x = event.position.x - state.position.x;
  event_viewport.position.y =
      state.position.y + state.size.y - event.position.y;
  viewport.viewport->mouse_event(state.size, event_viewport);
}

bool ViewportPtrSystem::scroll_event(
    ElementPtr element,
    const ScrollEvent& event) {
  const auto& state = element.state();
  const auto& viewport = element.viewport();

  ScrollEvent event_viewport = event;
  event_viewport.position.x = event.position.x - state.position.x;
  event_viewport.position.y =
      state.position.y + state.size.y - event.position.y;
  return viewport.viewport->scroll_event(state.size, event_viewport);
}

} // namespace datagui
