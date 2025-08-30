#include "datagui/element/checkbox.hpp"

namespace datagui {

void CheckboxSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  const auto& props = *e.props.cast<CheckboxProps>();

  e.fixed_size = Vecf::Constant(props.size);
  e.dynamic_size = Vecf::Zero();
  e.floating = false;
}

void CheckboxSystem::render(const Element& e, Renderer& renderer) {
  const auto& props = *e.props.cast<CheckboxProps>();

  renderer.queue_box(
      e.box(),
      props.bg_color,
      props.border_width,
      props.border_color,
      props.radius);

  if (!props.checked) {
    return;
  }

  Boxf icon_box;
  icon_box.lower = minimum(
      e.position + props.border_width.offset() + props.inner_padding.offset(),
      e.box().center());
  icon_box.upper = maximum(
      e.position + e.size - props.border_width.offset_opposite() -
          props.inner_padding.offset_opposite(),
      e.box().center());

  float icon_radius = 0;
  if (!e.box().empty() && props.radius > 0) {
    float size_ratio = std::min(
        icon_box.size().x / e.box().size().x,
        icon_box.size().y / e.box().size().y);
    icon_radius = std::max<float>(0, size_ratio * props.radius);
  }

  renderer
      .queue_box(icon_box, props.icon_color, 0, Color::Black(), icon_radius);
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
