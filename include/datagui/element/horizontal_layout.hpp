#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct HorizontalLayoutStyle {
  BoxStyle box;
  Direction direction;
  Alignment alignment;
  float inner_padding;

  void apply(const StyleManager& style) {
    box.apply(style);
    style.layout_direction(direction);
    style.layout_alignment(alignment);
    style.layout_inner_padding(inner_padding);
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
