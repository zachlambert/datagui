#pragma once

#include "datagui/color.hpp"
#include "datagui/layout.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/geometry_renderer.hpp"

namespace datagui {

struct LinearLayoutStyle {
  Direction direction = Direction::Vertical;
  Length length = literals::_wrap;
  Length width = literals::_wrap;
  BoxDims outer_padding = BoxDims();
  float inner_padding = 0;
  BoxDims border_width = BoxDims();
  Color border_color = Color::Black();
  Color bg_color = Color::Clear();
};

struct LinearLayoutElement {
  using Style = LinearLayoutStyle;
  Style style;
};

class LinearLayoutSystem : public ElementSystemBase<LinearLayoutElement> {
public:
  LinearLayoutSystem(GeometryRenderer& geometry_renderer) :
      geometry_renderer(geometry_renderer) {}

  void init(const std::function<void(LinearLayoutStyle&)> set_style);
  void set_layout_input(Tree::Ptr node) const override;
  void set_child_layout_output(Tree::Ptr node) const override;
  void render(Tree::ConstPtr node) const override;

private:
  GeometryRenderer& geometry_renderer;
};

} // namespace datagui
