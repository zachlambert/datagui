#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/font.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

struct TextBoxStyle : public BoxStyle, public SelectableTextStyle {};

struct TextBoxElement {
  using Style = TextBoxStyle;
  std::string text;
  Style style;
};

class TextBoxSystem : public ElementSystemBase<TextBoxElement> {
public:
  TextBoxSystem(FontManager& font_manager, TextRenderer& text_renderer) :
      font_manager(font_manager), text_renderer(text_renderer) {}

  void init(const std::function<void(TextBoxStyle&)> set_style);
  void set_layout_input(Tree::Ptr node) const override;
  void render(Tree::ConstPtr node) const override;

private:
  FontManager& font_manager;
  TextRenderer& text_renderer;
};

} // namespace datagui
