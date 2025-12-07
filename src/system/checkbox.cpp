#include "datagui/system/checkbox.hpp"

namespace datagui {

void CheckboxSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& checkbox = element.checkbox();

  checkbox.checkbox_size = fm->text_height(theme->text_font, theme->text_size) +
                           2 * theme->text_padding;

  state.fixed_size = Vec2::uniform(checkbox.checkbox_size);

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

  Box2 icon_box = state.box();
  Vec2 offset = Vec2::uniform(checkbox.checkbox_size * 0.2);
  icon_box.lower += offset;
  icon_box.upper -= offset;

  renderer.queue_box(icon_box, theme->input_color_bg_active);
}

void CheckboxSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& checkbox = element.checkbox();

  if (event.action == MouseAction::Release &&
      event.button == MouseButton::Left) {
    checkbox.checked = !checkbox.checked;
    if (checkbox.callback) {
      checkbox.callback(checkbox.checked);
    } else {
      element.set_dirty();
    }
  }
}

void CheckboxSystem::key_event(ElementPtr element, const KeyEvent& event) {
  auto& checkbox = element.checkbox();

  if (event.action == KeyAction::Release && event.key == Key::Enter) {
    checkbox.checked = !checkbox.checked;
    if (checkbox.callback) {
      checkbox.callback(checkbox.checked);
    } else {
      element.set_dirty();
    }
  }
}

} // namespace datagui
