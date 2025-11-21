#include "datagui/element/labelled.hpp"

namespace datagui {

void LabelledSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  const auto& props = *e.props.cast<LabelledProps>();

  Vecf label_size = fm->text_size(
                        props.label,
                        theme->text_font,
                        theme->text_size,
                        LengthWrap()) +
                    Vecf::Constant(2 * theme->text_padding);
  e.fixed_size = label_size;
  e.dynamic_size = Vecf::Zero();
  e.floating = false;

  if (children.empty()) {
    return;
  }
  const auto& child = *children[0];
  e.fixed_size.x += child.fixed_size.x + 2 * theme->layout_outer_padding;
  e.fixed_size.y = std::max(
      e.fixed_size.y,
      child.fixed_size.y + 2 * theme->layout_outer_padding);
  e.dynamic_size = child.dynamic_size;
}

void LabelledSystem::set_dependent_state(
    Element& e,
    const ElementList& children) {

  if (children.empty()) {
    return;
  }

  // Always hide excess children
  for (std::size_t i = 1; i < children.size(); i++) {
    children[i]->hidden = true;
  }

  auto& child = *children[0];

  const auto& props = *e.props.cast<LabelledProps>();

  Vecf label_size = fm->text_size(
                        props.label,
                        theme->text_font,
                        theme->text_size,
                        LengthWrap()) +
                    Vecf::Constant(2 * theme->text_padding);

  child.position.x = e.position.x + label_size.x + theme->layout_outer_padding;
  child.position.y = e.position.y + theme->layout_outer_padding;

  Vecf available_size = e.size;
  available_size.x -= label_size.x;
  available_size -= Vecf::Constant(2 * theme->layout_outer_padding);

  if (child.dynamic_size.x > 0) {
    child.size.x = available_size.x;
  } else {
    child.size.x = child.fixed_size.x;
  }
  if (child.dynamic_size.y > 0) {
    child.size.y = available_size.y;
  } else {
    child.size.y = child.fixed_size.y;
  }
}

void LabelledSystem::render(const Element& e, Renderer& renderer) {
  const auto& props = *e.props.cast<LabelledProps>();

  Vecf label_size = fm->text_size(
                        props.label,
                        theme->text_font,
                        theme->text_size,
                        LengthWrap()) +
                    Vecf::Constant(2 * theme->text_padding);

  Vecf label_position(
      e.position.x,
      e.position.y + e.size.y / 2 - label_size.y / 2);

  renderer.queue_box(
      Boxf(label_position, label_position + label_size),
      theme->input_color_bg_active,
      0,
      Color::Black(),
      0);
  renderer.queue_text(
      label_position + Vecf::Constant(theme->text_padding),
      props.label,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthWrap());
}

} // namespace datagui
