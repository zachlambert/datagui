#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct HorizontalLayoutStyle : public LinearLayoutStyle {
  AlignmentY vertical_alignment = AlignmentY::Top;
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
