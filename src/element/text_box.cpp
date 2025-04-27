#include "datagui/element/text_box.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

void TextBoxSystem::visit(
    Element element,
    const std::string& text,
    const SetTextBoxStyle& set_style) {
  auto& data = element.data<TextBoxData>();
  if (element.rerender()) {
    data.text = text;
    if (set_style) {
      set_style(data.style);
    }
  }
}

void TextBoxSystem::set_layout_input(Element element) const {
  const auto& data = element.data<TextBoxData>();
  const auto& style = data.style;

  element->fixed_size = (style.border_width + style.padding).size();
  element->dynamic_size = Vecf::Zero();
  element->hitbox_offset = Boxf();

  Vecf text_size = font_manager.text_size(data.text, style, style.width);
  element->fixed_size.y += text_size.y;

  if (auto width = std::get_if<LengthFixed>(&style.width)) {
    element->fixed_size.x += width->value;
  } else {
    element->fixed_size.x += text_size.x;
    if (auto width = std::get_if<LengthDynamic>(&style.width)) {
      element->dynamic_size.x = width->weight;
    }
  }
}

void TextBoxSystem::render(ConstElement element) const {
  const auto& data = element.data<TextBoxData>();
  const auto& style = data.style;

  text_renderer.queue_text(
      element->position,
      element->z_range.lower,
      data.text,
      style,
      style.width);
}

} // namespace datagui
