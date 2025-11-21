#include "datagui/system/text_box.hpp"

namespace datagui {

void TextBoxSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& text_box = element.text_box();

  state.fixed_size = fm->text_size(
                         text_box.text,
                         theme->text_font,
                         theme->text_size,
                         LengthWrap()) +
                     Vecf::Constant(2 * theme->text_padding);
  state.dynamic_size = Vecf::Zero();
  state.floating = false;
}

void TextBoxSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& text_box = element.text_box();

  Boxf mask;
  mask.lower = state.position + Vecf::Constant(theme->text_padding);
  mask.upper =
      state.position + state.size - Vecf::Constant(theme->text_padding);

  renderer.push_mask(mask);
  renderer.queue_text(
      state.position + Vecf::Constant(theme->text_padding),
      text_box.text,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthWrap());
  renderer.pop_mask();
}

} // namespace datagui
