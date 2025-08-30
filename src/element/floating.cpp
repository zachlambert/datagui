#include "datagui/element/floating.hpp"

namespace datagui {

void FloatingSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  e.fixed_size = Vecf::Zero();
  e.dynamic_size = Vecf::Zero();
  e.floating = true;
}

void FloatingSystem::set_dependent_state(
    Element& e,
    const ElementList& children) {
  auto& props = *e.props.cast<FloatingProps>();

  Vecf position = e.float_box.lower;
  Vecf size = e.float_box.size();
  if (props.title_bar_enable) {
    const auto& bar = props.title_bar;
    position.y += bar.border_width.size().y + bar.padding.size().y +
                  fm->text_height(props.text_style);
  }

  float fixed_size_y = 0;
  float dynamic_size_y = 0;
  for (auto child : children) {
    fixed_size_y += child->fixed_size.y;
    dynamic_size_y += child->dynamic_size.y;
  }

  float available_y = std::min(0.f, e.float_box.size().y - fixed_size_y);

  // Expected to only use one child, but allow for multiple children and
  // lay them out vertically without any padding, etc
  for (auto child : children) {
    child->position = position;
    if (child->dynamic_size.x > 0) {
      child->size.x = size.x;
    } else {
      child->size.x = child->fixed_size.x;
    }
    child->size.y = child->fixed_size.y;
    if (child->dynamic_size.y > 0) {
      child->size.y += available_y * child->dynamic_size.y / dynamic_size_y;
    }
    position.y += child->size.y;
  }

  if (auto type = std::get_if<FloatingTypeAbsolute>(&props.float_type)) {
    e.float_box.lower = e.layer_box.lower + type->margin.offset();
    e.float_box.upper = e.layer_box.upper - type->margin.offset_opposite();
  }
  if (auto type = std::get_if<FloatingTypeRelative>(&props.float_type)) {
    e.float_box.lower = e.box().lower + type->offset;
    e.float_box.upper = e.float_box.lower + type->size;
  }

  if (!props.title_bar_enable) {
    return;
  }
  const auto& bar = props.title_bar;
  auto& bar_box = props.title_bar_box;

  bar_box.lower = e.float_box.lower;
  bar_box.upper.x = e.float_box.upper.x;
  bar_box.upper.y = e.float_box.lower.y + fm->text_height(props.text_style) +
                    bar.border_width.size().y + bar.padding.size().y;

  props.title_bar_text_width =
      e.float_box.size().x - bar.border_width.size().x - bar.padding.size().x;

  if (!props.close_button_enable) {
    return;
  }
  const auto& button = props.close_button;
  auto& button_box = props.close_button_box;

  Vecf text_size = fm->text_size("close", props.text_style, LengthWrap());
  Vecf button_size = text_size + button.padding.size();
  props.title_bar_text_width -= (button_size.x + bar.padding.left);
  props.title_bar_text_width = std::max(0.f, props.title_bar_text_width);

  button_box.upper = bar_box.upper - bar.border_width.offset_opposite();
  button_box.lower = button_box.upper - button_size;
}

void FloatingSystem::render(const Element& e, Renderer& renderer) {
  auto& props = *e.props.cast<FloatingProps>();

  renderer.queue_box(e.float_box, props.bg_color, 0, Color::Black(), 0);

  if (!props.title_bar_enable) {
    return;
  }
  const auto& bar = props.title_bar;

  renderer.queue_box(
      props.title_bar_box,
      bar.bg_color,
      bar.border_width,
      bar.border_color,
      0);

  renderer.queue_text(
      e.float_box.lower + bar.border_width.offset() + bar.padding.offset(),
      props.title,
      props.text_style,
      LengthFixed(props.title_bar_text_width));

  if (!props.close_button_enable) {
    return;
  }

  renderer.queue_box(
      props.close_button_box,
      props.close_button.color,
      0,
      Color::Black(),
      0);

  renderer.queue_text(
      props.close_button_box.lower + bar.padding.offset(),
      "close",
      props.text_style,
      LengthWrap());
}

bool FloatingSystem::mouse_event(Element& e, const MouseEvent& event) {
  if (event.action != MouseAction::Release) {
    return false;
  }
  auto& props = *e.props.cast<FloatingProps>();
  if (!props.title_bar_enable) {
    return false;
  }

  if (!props.title_bar_enable || !props.close_button_enable) {
    return false;
  }

  if (!props.close_button_box.contains(event.position)) {
    return false;
  }
  props.open = false;
  props.open_changed = true;
  return true;
}

} // namespace datagui
