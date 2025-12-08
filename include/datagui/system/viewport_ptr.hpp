#pragma once

#include "datagui/element/system.hpp"

namespace datagui {

class ViewportPtrSystem : public System {
public:
  ViewportPtrSystem() {}
  void set_input_state(ElementPtr element) override;
  void render(ConstElementPtr element, GuiRenderer& renderer) override;
  void mouse_event(ElementPtr element, const MouseEvent& event) override;
};

} // namespace datagui
