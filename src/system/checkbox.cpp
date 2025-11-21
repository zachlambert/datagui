#include "datagui/system/checkbox.hpp"

namespace datagui {

void CheckboxSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();

  float size = fm->text_height(theme->text_font, theme->text_size);
  state.fixed_size = Vecf::Constant(size);
  state.dynamic_size = Vecf::Zero();
  state.floating = false;
}

void CheckboxSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& checkbox = element.checkbox();

  renderer.queue_box(
      state.box(),
      theme->input_color_bg,
      theme->input_border_width,
      theme->input_color_border,
      0);

  if (!checkbox.checked) {
    return;
  }

  Boxf icon_box;
  icon_box.lower = minimum(
      state.position + Vecf::Constant(theme->input_border_width * 2.f),
      state.box().center());
  icon_box.upper = maximum(
      state.position + state.size -
          Vecf::Constant(theme->input_border_width * 2.f),
      state.box().center());

  renderer
      .queue_box(icon_box, theme->input_color_bg_active, 0, Color::Black(), 0);
}

bool CheckboxSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  auto& checkbox = element.checkbox();

  if (event.action == MouseAction::Release &&
      event.button == MouseButton::Left) {
    checkbox.changed = true;
    checkbox.checked = !checkbox.checked;
    return true;
  }
  return false;
}

bool CheckboxSystem::key_event(ElementPtr element, const KeyEvent& event) {
  auto& checkbox = element.checkbox();

  if (event.action == KeyAction::Release && event.key == Key::Enter) {
    checkbox.changed = true;
    checkbox.checked = !checkbox.checked;
    return true;
  }
  return false;
}

} // namespace datagui
