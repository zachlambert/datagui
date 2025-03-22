#pragma once

#include "datagui/color.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/font.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

struct TextStyle {
  float max_width = 0;
  Font font = Font::DejaVuSans;
  int font_size = 24;
  Color text_color = Color::Black();
};

struct TextElement {
  using Style = TextStyle;
  std::string text;
  Style style;
};

class TextSystem : public ElementSystemBase<TextElement> {
public:
  TextSystem(FontManager& font_manager, TextRenderer& text_renderer) :
      font_manager(font_manager), text_renderer(text_renderer) {}

  void init(const std::function<void(TextStyle&)> set_style);
  void set_layout_input(Tree::Ptr node) const override;
  void render(Tree::ConstPtr node) const override;

private:
  FontManager& font_manager;
  TextRenderer& text_renderer;
};

} // namespace datagui
