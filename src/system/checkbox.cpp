#include "datagui/element/checkbox.hpp"

namespace datagui {

void CheckboxSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  const auto& props = *e.props.cast<CheckboxProps>();

  float size = fm->text_height(theme->text_font, theme->text_size);
  e.fixed_size = Vecf::Constant(size);
  e.dynamic_size = Vecf::Zero();
  e.floating = false;
}

void CheckboxSystem::render(const Element& e, Renderer& renderer) {
  const auto& props = *e.props.cast<CheckboxProps>();

  renderer.queue_box(
      e.box(),
      theme->input_color_bg,
      theme->input_border_width,
      theme->input_color_border,
      0);

  if (!props.checked) {
    return;
  }

  Boxf icon_box;
  icon_box.lower = minimum(
      e.position + Vecf::Constant(theme->input_border_width * 2.f),
      e.box().center());
  icon_box.upper = maximum(
      e.position + e.size - Vecf::Constant(theme->input_border_width * 2.f),
      e.box().center());

  renderer
      .queue_box(icon_box, theme->input_color_bg_active, 0, Color::Black(), 0);
}

bool CheckboxSystem::mouse_event(Element& e, const MouseEvent& event) {
  auto& props = *e.props.cast<CheckboxProps>();

  if (event.action == MouseAction::Release &&
      event.button == MouseButton::Left) {
    props.changed = true;
    props.checked = !props.checked;
    return true;
  }
  return false;
}

bool CheckboxSystem::key_event(Element& e, const KeyEvent& event) {
  auto& props = *e.props.cast<CheckboxProps>();

  if (event.action == KeyAction::Release && event.key == Key::Enter) {
    props.changed = true;
    props.checked = !props.checked;
    return true;
  }
  return false;
}

} // namespace datagui
