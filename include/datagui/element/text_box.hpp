#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct TextBoxStyle {
  BoxDims padding = 5;
  TextStyle text;

  void apply(const StyleManager& style) {
    style.text_padding(padding);
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

  void set_input_state(Element element) const override;
  void render(ConstElement element) const override;

private:
  Resources& res;
};

} // namespace datagui
