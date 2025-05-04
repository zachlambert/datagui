#include "datagui/element/checkbox.hpp"

namespace datagui {

const bool* CheckboxSystem::visit(
    Element element,
    const bool& initial_checked,
    const SetCheckboxStyle& set_style) {
  auto& data = element.data<CheckboxData>();
  if (element.is_new()) {
    data.checked = initial_checked;
    printf("Creating with checked = %i\n", data.checked);
  }
  if (element.rerender()) {
    if (set_style) {
      set_style(data.style);
    }
  }
  if (data.changed) {
    data.changed = false;
    return &data.checked;
  }
  return nullptr;
}

void CheckboxSystem::visit(
    Element element,
    const Variable<bool>& checked,
    const SetCheckboxStyle& set_style) {
  auto& data = element.data<CheckboxData>();

  if (element.is_new()) {
    data.checked = *checked;
  }
  if (element.rerender()) {
    if (set_style) {
      set_style(data.style);
    }
  }
  if (data.changed) {
    data.changed = false;
    checked.set(data.checked);
  } else if (checked.modified()) {
    data.checked = *checked;
  }
}

void CheckboxSystem::set_layout_input(Element element) const {
  const auto& data = element.data<CheckboxData>();
  const auto& style = data.style;

  element->fixed_size = Vecf::Constant(style.size);
  element->dynamic_size = Vecf::Zero();
  element->floating = false;
}

void CheckboxSystem::render(ConstElement element) const {
  const auto& data = element.data<CheckboxData>();
  const auto& style = data.style;

  geometry_renderer.queue_box(
      element->box(),
      style.bg_color,
      style.border_width,
      style.border_color,
      style.radius);

  if (!data.checked) {
    return;
  }

  Boxf checked_box;
  checked_box.lower = minimum(
      element->position + style.border_width.offset() +
          style.check_padding.offset(),
      element->box().center());
  checked_box.upper = maximum(
      element->position + element->size - style.border_width.offset_opposite() -
          style.check_padding.offset_opposite(),
      element->box().center());

  geometry_renderer
      .queue_box(checked_box, style.check_color, 0, Color::Black(), 0);
}

void CheckboxSystem::mouse_event(Element element, const MouseEvent& event) {
  auto& data = element.data<CheckboxData>();

  if (event.action == MouseAction::Release &&
      event.button == MouseButton::Left) {
    data.changed = true;
    data.checked = !data.checked;
    element.trigger();
  }
}

void CheckboxSystem::key_event(Element element, const KeyEvent& event) {
  auto& data = element.data<CheckboxData>();

  if (event.action == KeyAction::Release && event.key == Key::Enter) {
    data.changed = true;
    data.checked = !data.checked;
    element.trigger();
  }
}

} // namespace datagui
