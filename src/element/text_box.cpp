#include "datagui/element/text_box.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

void TextBoxSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  assert(children.empty());
  auto& props = *e.props.cast<TextBoxProps>();

  e.fixed_size = fm->text_size(
                     props.text,
                     theme->text_font,
                     theme->text_size,
                     LengthWrap()) +
                 Vecf::Constant(theme->text_padding);
  e.dynamic_size = Vecf::Zero();
  e.floating = false;
}

void TextBoxSystem::render(const Element& e, Renderer& renderer) {
  const auto& props = *e.props.cast<TextBoxProps>();

  Boxf mask;
  mask.lower = e.position + Vecf::Constant(theme->text_padding);
  mask.upper = e.position + e.size - Vecf::Constant(theme->text_padding);

  renderer.push_mask(mask);
  renderer.queue_text(
      e.position + Vecf::Constant(theme->text_padding),
      props.text,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthWrap());
  renderer.pop_mask();
}

} // namespace datagui
