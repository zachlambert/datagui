#pragma once

#include "datagui/input/text_selection.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/font.hpp"
#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

struct TextInputStyle : public BoxStyle, public SelectableTextStyle {
  Color focus_color = Color(0.0, 1.0, 1.0);

  TextInputStyle() {
    bg_color = Color::White();
    border_color = Color::Gray(0.5);
    border_width = 2;
    padding = 5;
    text_width = LengthDynamic(1.0);
  }
};

struct TextInputElement {
  using Style = TextInputStyle;
  Style style;
};

class TextInputSystem : public ElementSystemBase<TextInputElement> {
public:
  TextInputSystem(
      FontManager& font_manager,
      TextRenderer& text_renderer,
      GeometryRenderer& geometry_renderer) :
      font_manager(font_manager),
      text_renderer(text_renderer),
      geometry_renderer(geometry_renderer) {}

  void init(const std::function<void(TextInputStyle&)> set_style = nullptr);
  void set_layout_input(Tree::Ptr node) const override;
  void render(Tree::ConstPtr node) const override;

  void mouse_event(Tree::Ptr node, const MouseEvent& event) override;
  void key_event(Tree::Ptr node, const KeyEvent& event) override;
  void text_event(Tree::Ptr node, const TextEvent& event) override;

  void focus_enter(Tree::Ptr node) override;
  void focus_leave(Tree::Ptr node, bool success, Tree::ConstPtr new_node)
      override;

private:
  FontManager& font_manager;
  TextRenderer& text_renderer;
  GeometryRenderer& geometry_renderer;

  Tree::ConstPtr active_node;
  std::string active_text;
  TextSelection active_selection;
};

} // namespace datagui
