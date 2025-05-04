#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"
#include "datagui/visual/font.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

struct TextBoxStyle : public BoxStyle, public SelectableTextStyle {
  Length width = literals::_wrap;
};
using SetTextBoxStyle = SetStyle<TextBoxStyle>;

struct TextBoxData {
  using Style = TextBoxStyle;
  std::string text;
  Style style;
};

class TextBoxSystem : public ElementSystemImpl<TextBoxData> {
public:
  TextBoxSystem(FontManager& font_manager, TextRenderer& text_renderer) :
      font_manager(font_manager), text_renderer(text_renderer) {}

  void visit(
      Element element,
      const std::string& text,
      const SetTextBoxStyle& set_style);

  void set_layout_input(Element element) const override;
  void render(ConstElement element) const override;

private:
  FontManager& font_manager;
  TextRenderer& text_renderer;
};

} // namespace datagui
