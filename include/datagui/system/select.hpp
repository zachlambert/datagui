#pragma once

#include "datagui/element/system.hpp"
#include "datagui/render/font_registry.hpp"
#include "datagui/theme.hpp"

namespace dgui {

class SelectSystem : public System {
public:
  SelectSystem(
      std::shared_ptr<FontRegistry> font_registry,
      std::shared_ptr<Theme> theme) :
      font_registry(font_registry), theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void set_dependent_state(ElementPtr element) override;
  void render(ConstElementPtr element, DrawList& dl) override;
  void render_content(ConstElementPtr element, DrawList& dl) override;
  void mouse_event(ElementPtr element, const MouseEvent& event) override;
  void focus_enter(ElementPtr element) override;
  void focus_leave(ElementPtr element, bool success) override;

private:
  std::shared_ptr<FontRegistry> font_registry;
  std::shared_ptr<Theme> theme;
};

} // namespace dgui
