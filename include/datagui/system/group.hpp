#pragma once

#include "datagui/element/system.hpp"
#include "datagui/theme.hpp"

namespace datagui {

class GroupSystem : public System {
public:
  GroupSystem(std::shared_ptr<Theme> theme) : theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void set_dependent_state(ElementPtr element) override;
  void render(ConstElementPtr element, Renderer& renderer) override;
  bool scroll_event(ElementPtr element, const ScrollEvent& event) override;

private:
  std::shared_ptr<Theme> theme;
};

} // namespace datagui
