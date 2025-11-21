#include "datagui/system/labelled.hpp"

namespace datagui {

void LabelledSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& labelled = element.labelled();

  Vecf label_size = fm->text_size(
                        labelled.label,
                        theme->text_font,
                        theme->text_size,
                        LengthWrap()) +
                    Vecf::Constant(2 * theme->text_padding);
  state.fixed_size = label_size;
  state.dynamic_size = Vecf::Zero();
  state.floating = false;

  auto child = element.child();
  if (!child.exists()) {
    return;
  }
  state.fixed_size.x +=
      child.state().fixed_size.x + 2 * theme->layout_outer_padding;
  state.fixed_size.y = std::max(
      state.fixed_size.y,
      child.state().fixed_size.y + 2 * theme->layout_outer_padding);
  state.dynamic_size = child.state().dynamic_size;
}

void LabelledSystem::set_dependent_state(ElementPtr element) {
  const auto& state = element.state();
  const auto& labelled = element.labelled();

  auto child = element.child();
  if (!child.exists()) {
    return;
  }
  for (auto other = child.next(); other.exists(); other = other.next()) {
    other.state().hidden = true;
  }

  Vecf label_size = fm->text_size(
                        labelled.label,
                        theme->text_font,
                        theme->text_size,
                        LengthWrap()) +
                    Vecf::Constant(2 * theme->text_padding);

  child.state().position.x =
      state.position.x + label_size.x + theme->layout_outer_padding;
  child.state().position.y = state.position.y + theme->layout_outer_padding;

  Vecf available_size = state.size;
  available_size.x -= label_size.x;
  available_size -= Vecf::Constant(2 * theme->layout_outer_padding);

  if (child.state().dynamic_size.x > 0) {
    child.state().size.x = available_size.x;
  } else {
    child.state().size.x = child.state().fixed_size.x;
  }
  if (child.state().dynamic_size.y > 0) {
    child.state().size.y = available_size.y;
  } else {
    child.state().size.y = child.state().fixed_size.y;
  }
}

void LabelledSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& labelled = element.labelled();

  Vecf label_size = fm->text_size(
                        labelled.label,
                        theme->text_font,
                        theme->text_size,
                        LengthWrap()) +
                    Vecf::Constant(2 * theme->text_padding);

  Vecf label_position(
      state.position.x,
      state.position.y + state.size.y / 2 - label_size.y / 2);

  renderer.queue_box(
      Boxf(label_position, label_position + label_size),
      theme->input_color_bg_active,
      0,
      Color::Black(),
      0);
  renderer.queue_text(
      label_position + Vecf::Constant(theme->text_padding),
      labelled.label,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthWrap());
}

} // namespace datagui
