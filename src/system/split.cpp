#include "datagui/system/split.hpp"

namespace datagui {

void SplitSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& split = element.split();
}

void SplitSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& split = element.split();
}

void SplitSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& split = element.split();
}

} // namespace datagui
