#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct VerticalLayoutStyle {
  BoxStyle box;
  Direction direction = Direction::Vertical;
  Alignment alignment = Alignment::Center;
  float inner_padding = 0;

  void apply(const StyleManager& style) {
    box.apply(style);
    style.layout_direction(direction);
    style.layout_alignment(alignment);
    style.layout_inner_padding(inner_padding);
  }
};

struct VerticalLayoutData {
  VerticalLayoutStyle style;
  float overrun = 0;
  float scroll_pos = 0;
};

class VerticalLayoutSystem : public ElementSystemImpl<VerticalLayoutData> {
public:
  VerticalLayoutSystem(Resources& res) : res(res) {}

  void visit(Element element);

  void set_layout_input(Element element) const override;
  void set_child_layout_output(Element element) const override;
  void render(ConstElement element) const override;
  bool scroll_event(Element element, const ScrollEvent& event) override;

private:
  Resources& res;
};

} // namespace datagui
