#include "datagui/system/checkbox.hpp"

namespace datagui {

void CheckboxSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();

  float size = fm->text_height(theme->text_font, theme->text_size);
  state.fixed_size = Vec2::uniform(size);
  state.dynamic_size = Vec2();
  state.floating = false;
}

void CheckboxSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& checkbox = element.checkbox();

  renderer.queue_box(
      state.box(),
      theme->input_color_bg,
      theme->input_border_width,
      theme->input_color_border);

  if (!checkbox.checked) {
    return;
  }

  Box2 icon_box;
  icon_box.lower = minimum(
      state.position + Vec2::uniform(theme->input_border_width * 2.f),
      state.box().center());
  icon_box.upper = maximum(
      state.position + state.size -
          Vec2::uniform(theme->input_border_width * 2.f),
      state.box().center());

  renderer.queue_box(icon_box, theme->input_color_bg_active);
}

bool CheckboxSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  auto& checkbox = element.checkbox();

  if (event.action == MouseAction::Release &&
      event.button == MouseButton::Left) {
    checkbox.checked = !checkbox.checked;
    if (!checkbox.callback) {
      return true;
    }
    checkbox.callback(checkbox.checked);
  }
  return false;
}

bool CheckboxSystem::key_event(ElementPtr element, const KeyEvent& event) {
  auto& checkbox = element.checkbox();

  if (event.action == KeyAction::Release && event.key == Key::Enter) {
    checkbox.checked = !checkbox.checked;
    if (!checkbox.callback) {
      return true;
    }
    checkbox.callback(checkbox.checked);
  }
  return false;
}

} // namespace datagui
