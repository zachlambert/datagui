#pragma once

#include "datagui/element/system.hpp"
#include "datagui/theme.hpp"

namespace datagui {

class SplitSystem : public System {
public:
  SplitSystem(std::shared_ptr<Theme> theme) : theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void set_dependent_state(ElementPtr element) override;
  void render(ConstElementPtr element, Renderer& renderer) override;
  void mouse_event(ElementPtr element, const MouseEvent& event) override;

private:
  std::shared_ptr<Theme> theme;
};

} // namespace datagui
