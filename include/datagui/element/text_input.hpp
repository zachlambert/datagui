#pragma once

#include "datagui/input/text_selection.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"
#include "datagui/visual/font.hpp"
#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

struct TextInputStyle : public BoxStyle, public SelectableTextStyle {
  Color focus_color;
  Length width;

  TextInputStyle() {
    bg_color = Color::White();
    border_color = Color::Gray(0.5);
    border_width = 2;
    padding = 5;
    focus_color = Color(0.0, 1.0, 1.0);
    width = LengthDynamic(1.0);
  }
};
using SetTextInputStyle = SetStyle<TextInputStyle>;

struct TextInputData {
  using Style = TextInputStyle;
  std::string text;
  bool changed = false;
  Style style;
};

class TextInputSystem : public ElementSystemImpl<TextInputData> {
public:
  TextInputSystem(
      FontManager& font_manager,
      TextRenderer& text_renderer,
      GeometryRenderer& geometry_renderer) :
      font_manager(font_manager),
      text_renderer(text_renderer),
      geometry_renderer(geometry_renderer) {}

  const std::string* visit(
      Element element,
      const std::string& initial_text,
      const SetTextInputStyle& set_style);
  void visit(
      Element element,
      const Variable<std::string>& text,
      const SetTextInputStyle& set_style);

  void set_layout_input(Element element) const override;
  void render(ConstElement element) const override;

  void mouse_event(Element element, const MouseEvent& event) override;
  void key_event(Element element, const KeyEvent& event) override;
  void text_event(Element element, const TextEvent& event) override;

  void focus_enter(Element element) override;
  void focus_leave(Element element, bool success, ConstElement new_element)
      override;

private:
  FontManager& font_manager;
  TextRenderer& text_renderer;
  GeometryRenderer& geometry_renderer;

  std::string active_text;
  TextSelection active_selection;
};

} // namespace datagui
