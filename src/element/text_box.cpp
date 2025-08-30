#include "datagui/element/text_box.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

void TextBoxSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  assert(children.empty());
  auto& props = *e.props.cast<TextBoxProps>();

  e.fixed_size = fm->text_size(props.text, props.text_style, LengthWrap()) +
                 props.padding.size();
  e.dynamic_size = Vecf::Zero();
  e.floating = false;
}

void TextBoxSystem::render(const Element& e, Renderer& renderer) {
  const auto& props = *e.props.cast<TextBoxProps>();

  Boxf mask;
  mask.lower = e.position + props.padding.offset();
  mask.upper = e.position + e.size - props.padding.offset_opposite();

  renderer.push_mask(mask);
  renderer.queue_text(
      e.position + props.padding.offset(),
      props.text,
      props.text_style,
      LengthWrap());
  renderer.pop_mask();
}

} // namespace datagui
