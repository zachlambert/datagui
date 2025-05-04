#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"
#include "datagui/visual/geometry_renderer.hpp"

namespace datagui {

struct VerticalLayoutStyle : public LinearLayoutStyle {
  AlignmentX horizontal_alignment = AlignmentX::Left;
};
using SetVerticalLayoutStyle = SetStyle<VerticalLayoutStyle>;

struct VerticalLayoutData {
  using Style = VerticalLayoutStyle;
  Style style;
  float overrun = 0;
  float scroll_pos = 0;
};

class VerticalLayoutSystem : public ElementSystemImpl<VerticalLayoutData> {
public:
  VerticalLayoutSystem(GeometryRenderer& geometry_renderer) :
      geometry_renderer(geometry_renderer) {}

  void visit(Element element, const SetVerticalLayoutStyle& set_style);

  void set_layout_input(Element element) const override;
  void set_child_layout_output(Element element) const override;
  void render(ConstElement element) const override;
  bool scroll_event(Element element, const ScrollEvent& event) override;

private:
  GeometryRenderer& geometry_renderer;
};

} // namespace datagui
