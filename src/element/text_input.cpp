#include "datagui/element/text_input.hpp"
#include "datagui/log.hpp"

namespace datagui {

void TextInputSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  const auto& props = *e.props.cast<TextInputProps>();

  const std::string& text = e.focused ? active_text : props.text;
  e.fixed_size =
      2.f * Vecf::Constant(theme->input_border_width + theme->text_padding);
  e.dynamic_size = Vecf::Zero();
  e.floating = 0;

  Vecf text_size =
      fm->text_size(text, theme->text_font, theme->text_size, props.width);
  e.fixed_size.y += text_size.y;

  if (auto width = std::get_if<LengthFixed>(&props.width)) {
    e.fixed_size.x += width->value;
  } else {
    e.fixed_size.x += text_size.x;
    if (auto width = std::get_if<LengthDynamic>(&props.width)) {
      e.dynamic_size.x = width->weight;
    }
  }
}

void TextInputSystem::render(const Element& e, Renderer& renderer) {
  const auto& props = *e.props.cast<TextInputProps>();

  const std::string& text = e.focused ? active_text : props.text;

  Color border_color;
  if (e.in_focus_tree) {
    border_color = theme->input_color_border_focus;
  } else {
    border_color = theme->input_color_border;
  }

  renderer.queue_box(
      e.box(),
      theme->input_color_bg,
      theme->input_border_width,
      border_color,
      0);

  Vecf text_position =
      e.position +
      Vecf::Constant(theme->input_border_width + theme->text_padding);

  if (e.focused) {
    render_selection(
        text,
        text_position,
        active_selection,
        fm->font_structure(theme->text_font, theme->text_size),
        theme->text_cursor_color,
        theme->text_highlight_color,
        theme->text_cursor_width,
        props.width,
        renderer);
  }

  Boxf mask;
  mask.lower = e.position +
               Vecf::Constant(theme->input_border_width + theme->text_padding);
  mask.upper = e.position + e.size -
               Vecf::Constant(theme->input_border_width + theme->text_padding);

  renderer.push_mask(mask);
  renderer.queue_text(
      text_position,
      text,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      props.width);
  renderer.pop_mask();
}

bool TextInputSystem::mouse_event(Element& e, const MouseEvent& event) {
  const auto& props = *e.props.cast<TextInputProps>();

  Vecf text_origin =
      e.position +
      Vecf::Constant(theme->input_border_width + theme->text_padding);

  const auto& font = fm->font_structure(theme->text_font, theme->text_size);

  if (event.action == MouseAction::Press) {
    active_text = props.text;
  }

  std::size_t cursor_pos =
      find_cursor(font, active_text, props.width, event.position - text_origin);

  if (event.action == MouseAction::Press) {
    active_selection.reset(cursor_pos);
  } else if (event.action == MouseAction::Hold) {
    active_selection.end = cursor_pos;
  }

  return false;
}

bool TextInputSystem::key_event(Element& e, const KeyEvent& event) {
  auto& props = *e.props.cast<TextInputProps>();

  if (event.action == KeyAction::Press && event.key == Key::Enter) {
    if (props.text != active_text) {
      DATAGUI_LOG(
          "TextInputSystem::key_event",
          "Enter changed: %s -> %s",
          props.text.c_str(),
          active_text.c_str());
      props.text = active_text;
      props.changed = true;
      return true;
    }
    DATAGUI_LOG("TextInputSystem::key_event", "Enter unchanged");
    return false;
  }

  selection_key_event(active_text, active_selection, true, event);
  return false;
}

bool TextInputSystem::text_event(Element& element, const TextEvent& event) {
  selection_text_event(active_text, active_selection, true, event);
  return false;
}

void TextInputSystem::focus_enter(Element& e) {
  auto& props = *e.props.cast<TextInputProps>();

  active_selection.reset(0);
  active_text = props.text;
}

bool TextInputSystem::focus_leave(Element& e, bool success) {
  auto& props = *e.props.cast<TextInputProps>();
  if (success && props.text != active_text) {
    props.text = active_text;
    props.changed = true;
    return true;
  }
  return false;
}

} // namespace datagui
