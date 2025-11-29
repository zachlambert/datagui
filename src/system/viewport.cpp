#include "datagui/system/viewport.hpp"

namespace datagui {

void ViewportSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& viewport = element.viewport();

  state.fixed_size = Vec2(viewport.width, viewport.height);
  state.dynamic_size = Vec2();
}

void ViewportSystem::render(ConstElementPtr element, Renderer& renderer) {
  auto& state = element.state();
  const auto& viewport = element.viewport();
  viewport.viewport->render(state.box());
}

} // namespace datagui
