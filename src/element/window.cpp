#include "datagui/element/window.hpp"

namespace datagui {

void WindowSystem::visit(
    Element element,
    const std::string& title,
    const SetWindowStyle& set_style) {
  auto& data = element.data<WindowData>();
  if (element.rerender()) {
    data.title = title;
    if (set_style) {
      set_style(data.style);
    }
  }
}

void WindowSystem::set_layout_input(Element element) const {
  element->fixed_size = Vecf::Zero();
  element->dynamic_size = Vecf::Zero();
  element->floating = true;
}

void WindowSystem::set_child_layout_output(Element element) const {
  const auto& data = element.data<WindowData>();
  const auto& style = data.style;

  Vecf position = element->float_box.lower;
  Vecf size = element->float_box.size();
  if (style.title_bar) {
    position.y += style.title_bar->border_width.size().y +
                  style.title_bar->padding.size().y +
                  font_manager.text_height(*style.title_bar);
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

void WindowSystem::set_float_box(ConstElement window, Element element) const {
  const auto& data = element.data<WindowData>();
  const auto& style = data.style;
  if (auto position = std::get_if<WindowPositionAbsolute>(&style.position)) {
    element->float_box.lower = window->box().lower + position->margin.offset();
    element->float_box.upper =
        window->box().upper - position->margin.offset_opposite();
  }
  if (auto position = std::get_if<WindowPositionRelative>(&style.position)) {
    element->float_box.lower = element->box().lower + position->offset;
    element->float_box.upper = element->float_box.lower + position->size;
  }
}

void WindowSystem::render(ConstElement element) const {
  const auto& data = element.data<WindowData>();
  const auto& style = data.style;

  geometry_renderer
      .queue_box(element->float_box, style.bg_color, 0, Color::Black(), 0);

  if (!style.title_bar) {
    return;
  }
  const auto& bar = *style.title_bar;

  Boxf bar_box;
  bar_box.lower = element->float_box.lower;
  bar_box.upper.x = element->float_box.upper.x;
  bar_box.upper.y = element->float_box.lower.y + font_manager.text_height(bar) +
                    bar.border_width.size().y + bar.padding.size().y;

  geometry_renderer
      .queue_box(bar_box, bar.bg_color, bar.border_width, bar.border_color, 0);

  float title_width = element->float_box.size().x - bar.border_width.size().x -
                      bar.padding.size().x;

#if 1
  if (bar.close_button) {
    Vecf text_size = font_manager.text_size("close", bar, LengthWrap());
    Vecf button_size = text_size + bar.padding.size();
    title_width -= (button_size.x + bar.padding.left);
    title_width = std::max(0.f, title_width);

    Boxf button_box;
    button_box.upper = bar_box.upper - bar.border_width.offset_opposite();
    button_box.lower = button_box.upper - button_size;
    geometry_renderer
        .queue_box(button_box, bar.close_button_color, 0, Color::Black(), 0);

    text_renderer.queue_text(
        button_box.lower + bar.padding.offset(),
        "close",
        bar.font,
        bar.font_size,
        bar.text_color,
        LengthWrap());
  }

  text_renderer.queue_text(
      element->float_box.lower + bar.padding.offset(),
      data.title,
      bar.font,
      bar.font_size,
      bar.text_color,
      LengthFixed(title_width));
#endif
}

} // namespace datagui
