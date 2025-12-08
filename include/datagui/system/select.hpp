#pragma once

#include "datagui/element/system.hpp"
#include "datagui/theme.hpp"
#include "datagui/visual/font_manager.hpp"

namespace datagui {

class SelectSystem : public System {
public:
  SelectSystem(std::shared_ptr<FontManager> fm, std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void set_dependent_state(ElementPtr element) override;
  void render(ConstElementPtr element, GuiRenderer& renderer) override;
  void mouse_event(ElementPtr element, const MouseEvent& event) override;
  void focus_enter(ElementPtr element) override;
  void focus_leave(ElementPtr element, bool success) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
};

} // namespace datagui
