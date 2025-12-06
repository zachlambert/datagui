#pragma once

#include "datagui/element/system.hpp"
#include "datagui/theme.hpp"
#include "datagui/visual/font_manager.hpp"

namespace datagui {

class CollapsableSystem : public System {
public:
  CollapsableSystem(
      std::shared_ptr<FontManager> fm,
      std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void set_dependent_state(ElementPtr element) override;
  void render(ConstElementPtr element, Renderer& renderer) override;
  void mouse_event(ElementPtr element, const MouseEvent& event) override;
  bool scroll_event(ElementPtr element, const ScrollEvent& event) override;
  void key_event(ElementPtr element, const KeyEvent& event) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
};

} // namespace datagui
