#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct DropdownData {
  DropdownStyle style;
  std::vector<std::string> choices;
  int choice = -1;
  int choice_hovered = -1;
  bool changed = false;
  bool open = false;
};

class DropdownSystem : public ElementSystemImpl<DropdownData> {
public:
  DropdownSystem(Resources& res) : res(res) {}

  const int* visit(
      Element element,
      const std::vector<std::string>& choices,
      int initial_choice);
  void visit(
      Element element,
      const std::vector<std::string>& choices,
      const Variable<int>& choice);

  void set_layout_input(Element element) const override;
  void set_float_box(ConstElement root, Element element) const override;
  void render(ConstElement element) const override;
  void mouse_event(Element element, const MouseEvent& event) override;

  void focus_enter(Element element) override;
  void focus_leave(Element element, bool success, ConstElement new_element)
      override;

private:
  Resources& res;
};

} // namespace datagui
