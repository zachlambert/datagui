#pragma once

#include "datagui/color.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/font.hpp"

namespace datagui {

struct TextStyle {
  float max_width = -1;
  Font font = Font::DejaVuSans;
  int font_size = 12;
  Color text_color = Color::Black();
};

struct TextElement {
  using Style = TextStyle;
  std::string text;
  Style style;
};

class TextSystem : public ElementSystemBase<TextElement> {
public:
  TextSystem(const FontManager& font_manager) : font_manager(font_manager) {}

  void init(const std::function<void(TextStyle&)> set_style);
  void render(const State& state, Renderers& renderers) const override;

private:
  const FontManager& font_manager;
};

} // namespace datagui
