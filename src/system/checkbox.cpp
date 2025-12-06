#include "datagui/system/checkbox.hpp"

namespace datagui {

void CheckboxSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& checkbox = element.checkbox();

  checkbox.checkbox_size = fm->text_height(theme->text_font, theme->text_size) +
                           2 * theme->text_padding;

  state.fixed_size = Vec2::uniform(checkbox.checkbox_size);
  if (!checkbox.label.empty()) {
    checkbox.label_size = fm->text_size(
                              checkbox.label,
                              theme->text_font,
                              theme->text_size,
                              LengthWrap()) +
                          Vec2::uniform(2 * theme->text_padding);
    state.fixed_size.x = checkbox.label_size.x;
  } else {
    checkbox.label_size = Vec2();
  }

  state.dynamic_size = Vec2();
  state.floating = false;
}

void CheckboxSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& checkbox = element.checkbox();

  if (!checkbox.label.empty()) {
    renderer.queue_text(
        state.position + Vec2::uniform(theme->text_padding),
        checkbox.label,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthWrap());
  }

  Box2 checkbox_box = state.box();
  checkbox_box.lower.x += checkbox.label_size.x;
  renderer.queue_box(
      checkbox_box,
      theme->input_color_bg,
      theme->input_border_width,
      theme->input_color_border);

  if (!checkbox.checked) {
    return;
  }

  Box2 icon_box = checkbox_box;
  Vec2 offset = Vec2::uniform(
      std::min(checkbox.checkbox_size / 2, theme->input_border_width));
  icon_box.lower += offset;
  icon_box.upper -= offset;

  renderer.queue_box(icon_box, theme->input_color_bg_active);
}

void CheckboxSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& checkbox = element.checkbox();

  Box2 checkbox_box = state.box();
  checkbox_box.lower.x += checkbox.label_size.x;
  if (!checkbox_box.contains(event.position)) {
    return;
  }

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
