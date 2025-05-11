#include "datagui/element/floating.hpp"

namespace datagui {

void FloatingSystem::visit(
    Element element,
    Variable<bool> open,
    const std::string& title) {
  auto& data = element.data<FloatingData>();

  if (element.is_new()) {
    data.open = *open;
  }
  if (element.rerender()) {
    data.title = title;
    data.style.apply(res.style_manager);
  }
  if (data.open_changed) {
    data.open_changed = false;
    open.set(data.open);
  } else if (open.modified()) {
    data.open = *open;
  }
}

void FloatingSystem::set_input_state(Element element) const {
  element->fixed_size = Vecf::Zero();
  element->dynamic_size = Vecf::Zero();
  element->floating = true;
}

void FloatingSystem::set_dependent_state(Element element) const {
  auto& data = element.data<FloatingData>();
  const auto& style = data.style;

  Vecf position = element->float_box.lower;
  Vecf size = element->float_box.size();
  if (style.title_bar_enable) {
    const auto& bar = style.title_bar;
    position.y += bar.border_width.size().y + bar.padding.size().y +
                  res.font_manager.text_height(style.text);
  }

  float fixed_size_y = 0;
  float dynamic_size_y = 0;
  for (auto child = element.first_child(); child; child = child.next()) {
    fixed_size_y += child->fixed_size.y;
    dynamic_size_y += child->dynamic_size.y;
  }

  float available_y = std::min(0.f, element->float_box.size().y - fixed_size_y);

  // Expected to only use one child, but allow for multiple children and
  // lay them out vertically without any padding, etc
  for (auto child = element.first_child(); child; child = child.next()) {
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

  if (auto type = std::get_if<FloatingTypeAbsolute>(&style.float_type)) {
    element->float_box.lower = element->layer_box.lower + type->margin.offset();
    element->float_box.upper =
        element->layer_box.upper - type->margin.offset_opposite();
  }
  if (auto type = std::get_if<FloatingTypeRelative>(&style.float_type)) {
    element->float_box.lower = element->box().lower + type->offset;
    element->float_box.upper = element->float_box.lower + type->size;
  }

  if (!style.title_bar_enable) {
    return;
  }
  const auto& bar = style.title_bar;
  auto& bar_box = data.title_bar_box;

  bar_box.lower = element->float_box.lower;
  bar_box.upper.x = element->float_box.upper.x;
  bar_box.upper.y = element->float_box.lower.y +
                    res.font_manager.text_height(style.text) +
                    bar.border_width.size().y + bar.padding.size().y;

  res.geometry_renderer
      .queue_box(bar_box, bar.bg_color, bar.border_width, bar.border_color, 0);

  data.title_bar_text_width = element->float_box.size().x -
                              bar.border_width.size().x - bar.padding.size().x;

  if (!style.close_button_enable) {
    return;
  }
  const auto& button = style.close_button;
  auto& button_box = data.close_button_box;

  Vecf text_size =
      res.font_manager.text_size("close", style.text, LengthWrap());
  Vecf button_size = text_size + button.padding.size();
  data.title_bar_text_width -= (button_size.x + bar.padding.left);
  data.title_bar_text_width = std::max(0.f, data.title_bar_text_width);

  button_box.upper = bar_box.upper - bar.border_width.offset_opposite();
  button_box.lower = button_box.upper - button_size;
}

void FloatingSystem::render(ConstElement element) const {
  auto& data = element.data<FloatingData>();
  const auto& style = data.style;

  res.geometry_renderer
      .queue_box(element->float_box, style.bg_color, 0, Color::Black(), 0);

  if (!style.title_bar_enable) {
    return;
  }
  const auto& bar = style.title_bar;

  res.geometry_renderer.queue_box(
      data.title_bar_box,
      bar.bg_color,
      bar.border_width,
      bar.border_color,
      0);

  res.text_renderer.queue_text(
      element->float_box.lower + bar.border_width.offset() +
          bar.padding.offset(),
      data.title,
      style.text.font,
      style.text.font_size,
      style.text.text_color,
      LengthFixed(data.title_bar_text_width));

  if (!style.close_button_enable) {
    return;
  }

  res.geometry_renderer.queue_box(
      data.close_button_box,
      style.close_button.color,
      0,
      Color::Black(),
      0);

  res.text_renderer.queue_text(
      data.close_button_box.lower + bar.padding.offset(),
      "close",
      style.text.font,
      style.text.font_size,
      style.text.text_color,
      LengthWrap());
}

void FloatingSystem::mouse_event(Element element, const MouseEvent& event) {
  if (event.action != MouseAction::Release) {
    return;
  }
  auto& data = element.data<FloatingData>();
  const auto& style = data.style;
  if (!style.title_bar_enable) {
    return;
  }

  if (!style.title_bar_enable || !style.close_button_enable) {
    return;
  }

  if (!data.close_button_box.contains(event.position)) {
    return;
  }
  data.open = false;
  data.open_changed = true;
  element.trigger();
}

} // namespace datagui
