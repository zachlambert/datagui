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
  renderer.queue_image(state.box(), viewport.viewport->texture(), false);
}

void ViewportPtrSystem::mouse_event(
    ElementPtr element,
    const MouseEvent& event) {
  const auto& state = element.state();
  const auto& viewport = element.viewport();

  MouseEvent event_viewport = event;
  event_viewport.position = event.position - element.state().position;
  viewport.viewport->mouse_event(event_viewport);
}

} // namespace datagui
