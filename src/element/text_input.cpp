#include "datagui/element/text_input.hpp"

namespace datagui {

const std::string* TextInputSystem::visit(
    Element element,
    const std::string& initial_value) {
  auto& data = element.data<TextInputData>();
  if (element.is_new()) {
    data.text = initial_value;
  }
  if (element.rerender()) {
    res.style_manager.apply(data.style);
  }
  if (data.changed) {
    data.changed = false;
    return &data.text;
  }
  return nullptr;
}

void TextInputSystem::visit(
    Element element,
    const Variable<std::string>& text) {
  auto& data = element.data<TextInputData>();

  if (element.is_new()) {
    data.text = *text;
  }
  if (element.rerender()) {
    res.style_manager.apply(data.style);
  }
  if (data.changed) {
    data.changed = false;
    text.set(data.text);
  } else if (text.modified()) {
    data.text = *text;
  }
}

void TextInputSystem::set_layout_input(Element element) const {
  const auto& data = element.data<TextInputData>();
  const auto& style = data.style;

  element->fixed_size = (style.border_width + style.padding).size();
  element->dynamic_size = Vecf::Zero();
  element->floating = 0;

  Vecf text_size = res.font_manager.text_size(data.text, style, style.width);
  element->fixed_size.y += text_size.y;

  if (auto width = std::get_if<LengthFixed>(&style.width)) {
    element->fixed_size.x += width->value;
  } else {
    element->fixed_size.x += text_size.x;
    if (auto width = std::get_if<LengthDynamic>(&style.width)) {
      element->dynamic_size.x = width->weight;
    }
  }
}

void TextInputSystem::render(ConstElement element) const {
  const auto& data = element.data<TextInputData>();
  const auto& style = data.style;

  const std::string& text = element->focused ? active_text : data.text;

  res.geometry_renderer.queue_box(
      element->box(),
      style.bg_color,
      style.border_width,
      element->in_focus_tree ? style.focus_color : style.border_color,
      style.radius);

  Vecf text_position =
      element->position + (style.border_width + style.padding).offset();

  if (element->focused) {
    render_selection(
        res.font_manager.font_structure(style.font, style.font_size),
        style,
        style.width,
        text,
        text_position,
        active_selection,
        res.geometry_renderer);
  }

  res.text_renderer.queue_text(text_position, text, style, style.width);
}

void TextInputSystem::mouse_event(Element element, const MouseEvent& event) {
  const auto& data = element.data<TextInputData>();
  const auto& style = data.style;

  Vecf text_origin =
      element->position + (style.border_width + style.padding).offset();

  const auto& font =
      res.font_manager.font_structure(style.font, style.font_size);

  if (event.action == MouseAction::Press) {
    active_text = data.text;
  }

  std::size_t cursor_pos =
      find_cursor(font, active_text, style.width, event.position - text_origin);

  if (event.action == MouseAction::Press) {
    active_selection.reset(cursor_pos);
  } else if (event.action == MouseAction::Hold) {
    active_selection.end = cursor_pos;
  }
}

void TextInputSystem::key_event(Element element, const KeyEvent& event) {
  auto& data = element.data<TextInputData>();

  if (event.action == KeyAction::Press && event.key == Key::Enter) {
    if (data.text != active_text) {
      data.text = active_text;
      data.changed = true;
      element.trigger();
    }
    return;
  }
  selection_key_event(active_text, active_selection, true, event);
}

void TextInputSystem::text_event(Element element, const TextEvent& event) {
  selection_text_event(active_text, active_selection, true, event);
}

void TextInputSystem::focus_enter(Element element) {
  auto& data = element.data<TextInputData>();

  active_selection.reset(0);
  active_text = data.text;
}

void TextInputSystem::focus_leave(
    Element element,
    bool success,
    ConstElement new_element) {

  auto& data = element.data<TextInputData>();
  if (success && data.text != active_text) {
    data.text = active_text;
    data.changed = true;
    element.trigger();
  }
}

} // namespace datagui
