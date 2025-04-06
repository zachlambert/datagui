#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/geometry_renderer.hpp"

namespace datagui {

struct VerticalLayoutStyle : public LinearLayoutStyle {
  AlignmentX horizontal_alignment = AlignmentX::Left;
};

struct VerticalLayoutElement {
  using Style = VerticalLayoutStyle;
  Style style;
};

class VerticalLayoutSystem : public ElementSystemBase<VerticalLayoutElement> {
public:
  VerticalLayoutSystem(GeometryRenderer& geometry_renderer) :
      geometry_renderer(geometry_renderer) {}

  void init(const std::function<void(VerticalLayoutStyle&)> set_style);
  void set_layout_input(Tree::Ptr node) const override;
  void set_child_layout_output(Tree::Ptr node) const override;
  void render(Tree::ConstPtr node) const override;

private:
  GeometryRenderer& geometry_renderer;
};

} // namespace datagui
