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

void FloatingSystem::set_layout_input(Element element) const {
  element->fixed_size = Vecf::Zero();
  element->dynamic_size = Vecf::Zero();
  element->floating = true;
}

void FloatingSystem::set_child_layout_output(Element element) const {
  const auto& data = element.data<FloatingData>();
  const auto& style = data.style;

  Vecf position = element->float_box.lower;
  Vecf size = element->float_box.size();
  if (style.title_bar_enable) {
    const auto& bar = style.title_bar;
    position.y += bar.border_width.size().y + bar.padding.size().y +
                  res.font_manager.text_height(style.text);
  }

  // Expected to only use one child, but allow for multiple children and
  // lay them out vertically without any padding, etc
  for (auto child = element.first_child(); child; child = child.next()) {
    child->position = position;
    child->size.x = std::min(size.x, child->fixed_size.x);
    child->size.y = child->fixed_size.y;
    position.y += child->size.y;
  }
}

void FloatingSystem::set_float_box(ConstElement window, Element element) const {
  const auto& data = element.data<FloatingData>();
  const auto& style = data.style;
  if (auto type = std::get_if<FloatingTypeAbsolute>(&style.float_type)) {
    element->float_box.lower = window->box().lower + type->margin.offset();
    element->float_box.upper =
        window->box().upper - type->margin.offset_opposite();
  }
  if (auto type = std::get_if<FloatingTypeRelative>(&style.float_type)) {
    element->float_box.lower = element->box().lower + type->offset;
    element->float_box.upper = element->float_box.lower + type->size;
  }
}

void FloatingSystem::render(ConstElement element) const {
  const auto& data = element.data<FloatingData>();
  const auto& style = data.style;

  res.geometry_renderer
      .queue_box(element->float_box, style.bg_color, 0, Color::Black(), 0);

  if (!style.title_bar_enable) {
    return;
  }
  const auto& bar = style.title_bar;

  Boxf bar_box;
  bar_box.lower = element->float_box.lower;
  bar_box.upper.x = element->float_box.upper.x;
  bar_box.upper.y = element->float_box.lower.y +
                    res.font_manager.text_height(style.text) +
                    bar.border_width.size().y + bar.padding.size().y;

  res.geometry_renderer
      .queue_box(bar_box, bar.bg_color, bar.border_width, bar.border_color, 0);

  float title_width = element->float_box.size().x - bar.border_width.size().x -
                      bar.padding.size().x;

  if (style.close_button_enable) {
    const auto& button = style.close_button;

    Vecf text_size =
        res.font_manager.text_size("close", style.text, LengthWrap());
    Vecf button_size = text_size + button.padding.size();
    title_width -= (button_size.x + bar.padding.left);
    title_width = std::max(0.f, title_width);

    Boxf button_box;
    button_box.upper = bar_box.upper - bar.border_width.offset_opposite();
    button_box.lower = button_box.upper - button_size;
    res.geometry_renderer
        .queue_box(button_box, button.color, 0, Color::Black(), 0);

    res.text_renderer.queue_text(
        button_box.lower + bar.padding.offset(),
        "close",
        style.text.font,
        style.text.font_size,
        style.text.text_color,
        LengthWrap());
  }

  res.text_renderer.queue_text(
      element->float_box.lower + bar.padding.offset(),
      data.title,
      style.text.font,
      style.text.font_size,
      style.text.text_color,
      LengthFixed(title_width));
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

  const auto& bar = style.title_bar;

  float title_width = element->float_box.size().x - bar.border_width.size().x -
                      bar.padding.size().x;

  Vecf text_size =
      res.font_manager.text_size("close", style.text, LengthWrap());
  Vecf button_size = text_size + bar.padding.size();
  Vecf button_pos;
  button_pos.x =
      element->float_box.upper.x - bar.border_width.right - button_size.x;
  button_pos.y = element->float_box.lower.y + bar.border_width.top;

  if (!Boxf(button_pos, button_pos + button_size).contains(event.position)) {
    return;
  }
  data.open = false;
  data.open_changed = true;
  element.trigger();
}

} // namespace datagui
