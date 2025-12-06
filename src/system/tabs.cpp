#include "datagui/system/tabs.hpp"

namespace datagui {

void TabsSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& tabs = element.tabs();
}

void TabsSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& tabs = element.tabs();
}

void TabsSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& tabs = element.tabs();
}

void TabsSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  const auto& tabs = element.tabs();
}

} // namespace datagui
