#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/geometry_renderer.hpp"

namespace datagui {

struct HorizontalLayoutStyle : public LinearLayoutStyle {
  AlignmentY vertical_alignment = AlignmentY::Top;
};

struct HorizontalLayoutElement {
  using Style = HorizontalLayoutStyle;
  Style style;
};

class HorizontalLayoutSystem
    : public ElementSystemBase<HorizontalLayoutElement> {
public:
  HorizontalLayoutSystem(GeometryRenderer& geometry_renderer) :
      geometry_renderer(geometry_renderer) {}

  void init(const std::function<void(HorizontalLayoutStyle&)> set_style);
  void set_layout_input(Tree::Ptr node) const override;
  void set_child_layout_output(Tree::Ptr node) const override;
  void render(Tree::ConstPtr node) const override;

private:
  GeometryRenderer& geometry_renderer;
};

} // namespace datagui
