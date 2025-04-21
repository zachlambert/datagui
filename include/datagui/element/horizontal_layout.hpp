#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"
#include "datagui/visual/geometry_renderer.hpp"

namespace datagui {

struct HorizontalLayoutStyle : public LinearLayoutStyle {
  AlignmentY vertical_alignment = AlignmentY::Top;
};
using SetHorizontalLayoutStyle = SetStyle<HorizontalLayoutStyle>;

struct HorizontalLayoutData {
  using Style = HorizontalLayoutStyle;
  Style style;
};

class HorizontalLayoutSystem : public ElementSystemImpl<HorizontalLayoutData> {
public:
  HorizontalLayoutSystem(GeometryRenderer& geometry_renderer) :
      geometry_renderer(geometry_renderer) {}

  void visit(Element element, const SetHorizontalLayoutStyle& set_style);

  void set_layout_input(Element element) const override;
  void set_child_layout_output(Element elment) const override;
  void render(ConstElement element) const override;

private:
  GeometryRenderer& geometry_renderer;
};

} // namespace datagui
