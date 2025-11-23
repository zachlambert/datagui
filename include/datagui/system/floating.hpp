#pragma once

#include "datagui/system/system.hpp"
#include "datagui/theme.hpp"
#include <optional>

namespace datagui {

class FloatingSystem : public System {
public:
  FloatingSystem(
      std::shared_ptr<FontManager> fm,
      std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void set_dependent_state(ElementPtr element) override;
  void render(ConstElementPtr element, Renderer& renderer) override;
  bool mouse_event(ElementPtr element, const MouseEvent& event) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
};

} // namespace datagui
