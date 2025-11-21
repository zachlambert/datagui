#pragma once

#include "datagui/system/system.hpp"
#include "datagui/theme.hpp"
#include "datagui/visual/font_manager.hpp"

namespace datagui {

class LabelledSystem : public System {
public:
  LabelledSystem(
      std::shared_ptr<FontManager> fm,
      std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void set_dependent_state(ElementPtr element) override;
  void render(ConstElementPtr element, Renderer& renderer) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
};

} // namespace datagui
