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
  // Title bar size
  position.y += 2 * theme->input_border_width + 2 * theme->text_padding +
                fm->text_height(theme->text_font, theme->text_size);

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

  auto& bar_box = props.title_bar_box;
  bar_box.lower = e.float_box.lower;
  bar_box.upper.x = e.float_box.upper.x;
  bar_box.upper.y = e.float_box.lower.y +
                    fm->text_height(theme->text_font, theme->text_size) +
                    2.f * (theme->input_border_width + theme->text_padding);

  props.title_bar_text_width =
      e.float_box.size().x -
      2.f * (theme->input_border_width + theme->text_padding);

  auto& button_box = props.close_button_box;
  Vecf text_size =
      fm->text_size("close", theme->text_font, theme->text_size, LengthWrap());
  Vecf button_size = text_size + 2.f * Vecf::Constant(theme->text_padding);
  props.title_bar_text_width -= (button_size.x + theme->text_padding);
  props.title_bar_text_width = std::max(0.f, props.title_bar_text_width);

  button_box.upper = bar_box.upper - Vecf::Constant(theme->input_border_width);
  button_box.lower = button_box.upper - button_size;
}

void FloatingSystem::render(const Element& e, Renderer& renderer) {
  auto& props = *e.props.cast<FloatingProps>();

  renderer.queue_box(e.float_box, theme->layout_color_bg, 0, Color::Black(), 0);

  renderer.queue_box(
      props.title_bar_box,
      theme->layout_color_bg,
      theme->input_border_width,
      theme->input_color_border,
      0);

  renderer.queue_text(
      e.float_box.lower +
          Vecf::Constant(theme->input_border_width + theme->text_padding),
      props.title,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthFixed(props.title_bar_text_width));

  renderer.queue_box(
      props.close_button_box,
      theme->input_color_bg,
      0,
      Color::Black(),
      0);

  renderer.queue_text(
      props.close_button_box.lower + Vecf::Constant(theme->text_padding),
      "close",
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthWrap());
}

bool FloatingSystem::mouse_event(Element& e, const MouseEvent& event) {
  if (event.action != MouseAction::Release) {
    return false;
  }
  auto& props = *e.props.cast<FloatingProps>();

  if (!props.close_button_box.contains(event.position)) {
    return false;
  }
  props.open = false;
  props.open_changed = true;
  return true;
}

} // namespace datagui
