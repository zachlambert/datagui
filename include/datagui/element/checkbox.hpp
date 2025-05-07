#pragma once

#include "datagui/input/text_selection.hpp"
#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct CheckboxStyle {
  Color bg_color = Color::Gray(0.8);
  Color border_color = Color::Gray(0.2);
  Color check_color = Color::Gray(0.2);
  BoxDims border_width = 2;
  BoxDims check_padding = 2;
  float radius = 0;
  float size = 24;
};

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
