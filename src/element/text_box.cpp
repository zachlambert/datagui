#include "datagui/element/text_box.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

void TextBoxElement::set_input_state(
    const std::vector<const Element*>& children) {
  fixed_size =
      resources->font_manager.text_size(text, style.text, LengthWrap()) +
      style.padding.size();
  dynamic_size = Vecf::Zero();
  floating = false;
}

void TextBoxElement::render() const {
  Boxf mask;
  mask.lower = position + style.padding.offset();
  mask.upper = position + size - style.padding.offset_opposite();

  resources->text_renderer.push_mask(mask);
  resources->text_renderer.queue_text(
      position + style.padding.offset(),
      text,
      style.text,
      LengthWrap());
  resources->text_renderer.pop_mask();
}

} // namespace datagui
