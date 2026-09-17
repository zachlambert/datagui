#pragma once

#include "datagui/element/system.hpp"

namespace dgui {

// WidgetPtr = Arbitrary content put inside a fixed or dynamically sized box
// Widget = The class defining what is drawn inside the box
class WidgetPtrSystem : public System {
public:
  WidgetPtrSystem(std::shared_ptr<Theme> theme) : theme(theme) {}
  void set_input_state(ElementPtr element) override;
  void set_dependent_state(ElementPtr element) override;
  void render(ConstElementPtr element, DrawList& dl) override;
  void mouse_event(ElementPtr element, const MouseEvent& event) override;
  bool scroll_event(ElementPtr element, const ScrollEvent& event) override;

private:
  std::shared_ptr<Theme> theme;
};

} // namespace dgui
