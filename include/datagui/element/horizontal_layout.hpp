#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct HorizontalLayoutStyle {
  BoxStyle box;
  Alignment alignment = Alignment::Center;

  void apply(const StyleManager& style) {
    style.layout_length(box.width);
    style.layout_width(box.height);
    style.layout_padding(box.padding);
    style.layout_bg_color(box.bg_color);
    box.border_width = 0;
    box.border_color = Color::Black();
    box.radius = 0;
    style.layout_alignment(alignment);
  }
};

struct HorizontalLayoutData {
  HorizontalLayoutStyle style;
};

class HorizontalLayoutSystem : public ElementSystemImpl<HorizontalLayoutData> {
public:
  HorizontalLayoutSystem(Resources& res) : res(res) {}

  void visit(Element element);

  void set_layout_input(Element element) const override;
  void set_child_layout_output(Element elment) const override;
  void render(ConstElement element) const override;

private:
  Resources& res;
};

} // namespace datagui
