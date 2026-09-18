#pragma once

#include "datagui/element/system.hpp"
#include "datagui/render/font_registry.hpp"
#include "datagui/theme.hpp"

namespace dgui {

class TextBoxSystem : public System {
public:
  TextBoxSystem(
      std::shared_ptr<FontRegistry> font_registry,
      std::shared_ptr<Theme> theme) :
      font_registry(font_registry), theme(theme) {}
  void set_input_state(ElementPtr element) override;
  void render(ConstElementPtr element, DrawList& dl) override;

private:
  std::shared_ptr<FontRegistry> font_registry;
  std::shared_ptr<Theme> theme;
};

} // namespace dgui
