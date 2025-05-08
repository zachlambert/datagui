#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct FloatingData {
  FloatingStyle style;
  std::string title;
  bool open = false;
  bool open_changed = false;
};

class FloatingSystem : public ElementSystemImpl<FloatingData> {
public:
  FloatingSystem(Resources& res) : res(res) {}

  void visit(Element element, Variable<bool> open, const std::string& title);

  void set_layout_input(Element element) const override;
  void set_child_layout_output(Element elment) const override;
  void set_float_box(ConstElement window, Element element) const override;
  void render(ConstElement element) const override;
  void mouse_event(Element element, const MouseEvent& event) override;

private:
  Resources& res;
};

} // namespace datagui
