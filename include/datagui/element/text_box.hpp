#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct TextBoxStyle {
  BoxStyle box;
  TextStyle text;

  void apply(const StyleManager& style) {
    // TODO: Does this need the full BoxStyle struct?
    box.width = LengthWrap();
    box.height = LengthWrap();
    style.text_padding(box.padding);
    box.bg_color = Color::Clear();
    box.border_width = 0;
    box.border_color = Color::Black();
    box.radius = 0;
    text.apply(style);
  }
};

struct TextBoxData {
  TextBoxStyle style;
  std::string text;
};

class TextBoxSystem : public ElementSystemImpl<TextBoxData> {
public:
  TextBoxSystem(Resources& res) : res(res) {}

  void visit(Element element, const std::string& text);

  void set_layout_input(Element element) const override;
  void render(ConstElement element) const override;

private:
  Resources& res;
};

} // namespace datagui
