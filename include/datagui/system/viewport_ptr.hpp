#pragma once

#include "datagui/element/system.hpp"

namespace dgui {

class ViewportPtrSystem : public System {
public:
  ViewportPtrSystem(std::shared_ptr<Theme> theme) : theme(theme) {}
  void set_input_state(ElementPtr element) override;
  void set_dependent_state(ElementPtr element) override;
  void render(ConstElementPtr element, GuiRenderer& renderer) override;
  void mouse_event(ElementPtr element, const MouseEvent& event) override;
  bool scroll_event(ElementPtr element, const ScrollEvent& event) override;

private:
  std::shared_ptr<Theme> theme;
};

} // namespace dgui
