#include "datagui/system/viewport_ptr.hpp"

namespace dgui {

void ViewportPtrSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& viewport = element.viewport();

  state.fixed_size = Vec2();
  state.dynamic_size = Vec2();
  state.dynamic_y_size = 0;

  if (auto value = std::get_if<LengthFixed>(&viewport.width)) {
    state.fixed_size.x = value->value;
  } else if (auto value = std::get_if<LengthDynamic>(&viewport.width)) {
    state.dynamic_size.x = value->weight;
  } else {
    throw std::runtime_error("Cannot set wrap length for viewport width");
  }

  if (auto value = std::get_if<LengthFixed>(&viewport.height)) {
    state.fixed_size.y = value->value;
  } else if (auto value = std::get_if<LengthDynamic>(&viewport.height)) {
    if (state.dynamic_size.x > 0) {
      state.dynamic_y_size = 1.f / viewport.viewport->get_aspect_ratio();
    } else {
      state.dynamic_size.y = value->weight;
    }
  } else {
    throw std::runtime_error("Cannot set wrap length for viewport height");
  }

  state.floating = false;
}

void ViewportPtrSystem::render(ConstElementPtr element, GuiRenderer& renderer) {
  const auto& state = element.state();
  const auto& viewport = element.viewport();
  renderer.queue_viewport(
      Box2(state.position, state.position + state.size),
      viewport.viewport.get());
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
