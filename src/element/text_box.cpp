#include "datagui/element/text_box.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

void TextBoxSystem::visit(Element element, const std::string& text) {
  auto& data = element.data<TextBoxData>();
  if (element.rerender()) {
    data.text = text;
    data.style.apply(res.style_manager);
  }
}

void TextBoxSystem::set_layout_input(Element element) const {
  const auto& data = element.data<TextBoxData>();
  const auto& style = data.style;

  element->fixed_size =
      res.font_manager.text_size(data.text, style.text, LengthWrap()) +
      style.padding.size();
  element->dynamic_size = Vecf::Zero();
  element->floating = false;
}

void TextBoxSystem::render(ConstElement element) const {
  const auto& data = element.data<TextBoxData>();
  const auto& style = data.style;

  res.text_renderer
      .queue_text(element->position, data.text, style.text, LengthWrap());
}

} // namespace datagui
