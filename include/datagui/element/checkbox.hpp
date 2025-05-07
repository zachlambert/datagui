#pragma once

#include "datagui/input/text_selection.hpp"
#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct CheckboxData {
  CheckboxStyle style;
  bool checked = false;
  bool changed = false;
};

class CheckboxSystem : public ElementSystemImpl<CheckboxData> {
public:
  CheckboxSystem(Resources& res) : res(res) {}

  const bool* visit(Element element, const bool& initial_checked);
  void visit(Element element, const Variable<bool>& checked);

  void set_layout_input(Element element) const override;
  void render(ConstElement element) const override;

  void mouse_event(Element element, const MouseEvent& event) override;
  void key_event(Element element, const KeyEvent& event) override;

private:
  Resources& res;

  std::string active_text;
  TextSelection active_selection;
};

} // namespace datagui
