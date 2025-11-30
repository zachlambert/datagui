#pragma once

#include "datagui/system/system.hpp"

namespace datagui {

class ViewportSystem : public System {
public:
  ViewportSystem() {}
  void set_input_state(ElementPtr element) override;
  void render(ConstElementPtr element, Renderer& renderer) override;
  bool mouse_event(ElementPtr element, const MouseEvent& event) override;
};

} // namespace datagui
