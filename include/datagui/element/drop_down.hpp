#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"
#include "datagui/visual/font.hpp"
#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

struct DropDownStyle : public BoxStyle, public TextStyle {
  Length content_width = LengthWrap();
  float inner_border_width = 0;
  Color choice_color = Color::Gray(0.9);
};
using SetDropDownStyle = SetStyle<DropDownStyle>;

struct DropDownData {
  std::vector<std::string> choices;
  int choice = -1;
  int choice_hovered = -1;
  bool changed = false;
  bool open = false;
  DropDownStyle style;
};

class DropDownSystem : public ElementSystemImpl<DropDownData> {
public:
  DropDownSystem(
      FontManager& font_manager,
      TextRenderer& text_renderer,
      GeometryRenderer& geometry_renderer) :
      font_manager(font_manager),
      text_renderer(text_renderer),
      geometry_renderer(geometry_renderer) {}

  const int* visit(
      Element element,
      const std::vector<std::string>& choices,
      int initial_choice,
      const SetDropDownStyle& set_style);
  void visit(
      Element element,
      const std::vector<std::string>& choices,
      const Variable<int>& choice,
      const SetDropDownStyle& set_style);

  void set_layout_input(Element element) const override;
  void set_float_box(ConstElement root, Element element) const override;
  void render(ConstElement element) const override;
  void mouse_event(Element element, const MouseEvent& event) override;

  void focus_enter(Element element) override;
  void focus_leave(Element element, bool success, ConstElement new_element)
      override;

private:
  FontManager& font_manager;
  TextRenderer& text_renderer;
  GeometryRenderer& geometry_renderer;
};

} // namespace datagui
