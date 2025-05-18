#include "datagui/element/checkbox.hpp"

namespace datagui {

const bool* CheckboxSystem::visit(
    Element element,
    const bool& initial_checked) {
  auto& data = element.data<CheckboxData>();
  if (element.is_new()) {
    data.checked = initial_checked;
  }
  if (element.rerender()) {
    data.style.apply(res.style_manager);
  }
  if (data.changed) {
    data.changed = false;
    return &data.checked;
  }
  return nullptr;
}

void CheckboxSystem::visit(Element element, const Variable<bool>& checked) {
  auto& data = element.data<CheckboxData>();

  if (element.is_new()) {
    data.checked = *checked;
  }
  if (element.rerender()) {
    data.style.apply(res.style_manager);
  }
  if (data.changed) {
    data.changed = false;
    checked.set(data.checked);
  } else if (checked.modified()) {
    data.checked = *checked;
  }
}

void CheckboxSystem::write(Element element, bool checked) {
  auto& data = element.data<CheckboxData>();
  if (element.rerender()) {
    data.style.apply(res.style_manager);
  }
  data.checked = checked;
}

bool read(ConstElement element) {
  const auto& data = element.data<CheckboxData>();
  return data.checked;
}

void CheckboxSystem::set_input_state(Element element) const {
  const auto& data = element.data<CheckboxData>();
  const auto& style = data.style;

  element->fixed_size = Vecf::Constant(style.size);
  element->dynamic_size = Vecf::Zero();
  element->floating = false;
}

void CheckboxSystem::render(ConstElement element) const {
  const auto& data = element.data<CheckboxData>();
  const auto& style = data.style;

  res.geometry_renderer.queue_box(
      element->box(),
      style.bg_color,
      style.border_width,
      style.border_color,
      style.radius);

  if (!data.checked) {
    return;
  }

  Boxf icon_box;
  icon_box.lower = minimum(
      element->position + style.border_width.offset() +
          style.inner_padding.offset(),
      element->box().center());
  icon_box.upper = maximum(
      element->position + element->size - style.border_width.offset_opposite() -
          style.inner_padding.offset_opposite(),
      element->box().center());

  float icon_radius = 0;
  if (!element->box().empty() && style.radius > 0) {
    float size_ratio = std::min(
        icon_box.size().x / element->box().size().x,
        icon_box.size().y / element->box().size().y);
    icon_radius = std::max<float>(0, size_ratio * style.radius);
  }

  res.geometry_renderer
      .queue_box(icon_box, style.icon_color, 0, Color::Black(), icon_radius);
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
