#include "datagui/element/drop_down.hpp"

namespace datagui {

const int* DropDownSystem::visit(
    Element element,
    const std::vector<std::string>& choices,
    int initial_choice,
    const SetDropDownStyle& set_style) {

  auto& data = element.data<DropDownData>();
  if (element.is_new()) {
    data.choices = choices;
    data.choice = initial_choice;
  }
  if (element.rerender()) {
    if (set_style) {
      set_style(data.style);
    }
  }
  if (data.changed) {
    data.changed = false;
    return &data.choice;
  }
  return nullptr;
}

void DropDownSystem::visit(
    Element element,
    const std::vector<std::string>& choices,
    const Variable<int>& choice,
    const SetDropDownStyle& set_style) {
  auto& data = element.data<DropDownData>();

  if (element.is_new()) {
    data.choice = *choice;
  }
  if (element.rerender()) {
    if (set_style) {
      set_style(data.style);
    }
  }
  if (data.changed) {
    data.changed = false;
    choice.set(data.choice);
  } else if (choice.modified()) {
    data.choice = *choice;
  }
}

void DropDownSystem::set_layout_input(Element element) const {
  const auto& data = element.data<TextInputData>();
  const auto& style = data.style;

  element->fixed_size = (style.border_width + style.padding).size();
  element->dynamic_size = Vecf::Zero();

  Vecf text_size = font_manager.text_size(data.text, style);
  element->fixed_size.y += text_size.y;

  if (auto width = std::get_if<LengthFixed>(&style.text_width)) {
    element->fixed_size.x += width->value;
  } else {
    element->fixed_size.x += text_size.x;
    if (auto width = std::get_if<LengthDynamic>(&style.text_width)) {
      element->dynamic_size.x = width->weight;
    }
  }
}

void DropDownSystem::render(ConstElement element) const {
  const auto& data = element.data<TextInputData>();
  const auto& style = data.style;

  const std::string& text = element->focused ? active_text : data.text;

  geometry_renderer.queue_box(
      element->box(),
      style.bg_color,
      style.border_width,
      element->in_focus_tree ? style.focus_color : style.border_color,
      style.radius);

  Vecf text_position =
      element->position + (style.border_width + style.padding).offset();

  if (element->focused) {
    render_selection(
        font_manager.font_structure(style.font, style.font_size),
        style,
        text,
        text_position,
        active_selection,
        geometry_renderer);
  }

  text_renderer.queue_text(text_position, text, style);
}

void DropDownSystem::mouse_event(Element element, const MouseEvent& event) {
  const auto& data = element.data<TextInputData>();
  const auto& style = data.style;

  Vecf text_origin =
      element->position + (style.border_width + style.padding).offset();

  const auto& font = font_manager.font_structure(style.font, style.font_size);

  if (event.action == MouseAction::Press) {
    active_text = data.text;
  }

  std::size_t cursor_pos = find_cursor(
      font,
      active_text,
      style.text_width,
      event.position - text_origin);

  if (event.action == MouseAction::Press) {
    active_selection.reset(cursor_pos);
  } else if (event.action == MouseAction::Hold) {
    active_selection.end = cursor_pos;
  }
}

void DropDownSystem::key_event(Element element, const KeyEvent& event) {
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

void DropDownSystem::text_event(Element element, const TextEvent& event) {
  selection_text_event(active_text, active_selection, true, event);
}

void DropDownSystem::focus_enter(Element element) {
  auto& data = element.data<TextInputData>();

  active_selection.reset(0);
  active_text = data.text;
}

void DropDownSystem::focus_leave(
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
