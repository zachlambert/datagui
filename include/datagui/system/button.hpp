#pragma once

#include "datagui/element/system.hpp"
#include "datagui/render/font_registry.hpp"
#include "datagui/theme.hpp"
#include <assert.h>

namespace dgui {

class ButtonSystem : public System {
public:
  ButtonSystem(
      std::shared_ptr<FontRegistry> font_registry,
      std::shared_ptr<Theme> theme) :
      font_registry(font_registry), theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void render(ConstElementPtr element, DrawList& dl) override;
  void mouse_event(ElementPtr element, const MouseEvent& event) override;
  void key_event(ElementPtr element, const KeyEvent& event) override;

private:
  std::shared_ptr<FontRegistry> font_registry;
  std::shared_ptr<Theme> theme;
};

} // namespace dgui
