#pragma once

#include "datagui/color.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/geometry_renderer.hpp"

namespace datagui {

enum class LinearLayoutDirection { Horizontal, Vertical };

struct LinearLayoutStyle {
  LinearLayoutDirection direction = LinearLayoutDirection::Vertical;
  float length = 0;
  float width = -1;
  float padding = 0;
  float inner_padding = 0;
  float border_width = 0;
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
