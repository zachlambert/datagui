#include "datagui/system/dropdown.hpp"

namespace datagui {

void DropdownSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& tabs = element.tabs();
}

void DropdownSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& tabs = element.tabs();
}

void DropdownSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& tabs = element.tabs();
}

void DropdownSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  const auto& tabs = element.tabs();
}

bool DropdownSystem::focus_leave(ElementPtr element, bool success) {
  return true;
}

} // namespace datagui
