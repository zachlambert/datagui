#include "datagui/element/text_input.hpp"
#include "datagui/log.hpp"

namespace datagui {

void TextInputSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  const auto& props = *e.props.cast<TextInputProps>();

  e.fixed_size = props.border_width.size() + props.padding.size();
  e.dynamic_size = Vecf::Zero();
  e.floating = 0;

  Vecf text_size = fm->text_size(props.text, props.text_style, props.width);
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
    border_color =
        props.border_color.multiply(props.input_style.focus_color_factor);
  } else {
    border_color = props.border_color;
  }

  renderer
      .queue_box(e.box(), props.bg_color, props.border_width, border_color, 0);

  Vecf text_position =
      e.position + props.border_width.offset() + props.padding.offset();

  if (e.focused) {
    render_selection(
        fm->font_structure(props.text_style.font, props.text_style.font_size),
        props.text_style,
        props.width,
        text,
        text_position,
        active_selection,
        renderer);
  }

  Boxf mask;
  mask.lower =
      e.position + props.padding.offset() + props.border_width.offset();
  mask.upper = e.position + e.size - props.padding.offset_opposite() -
               props.border_width.offset_opposite();

  renderer.push_mask(mask);
  renderer.queue_text(text_position, text, props.text_style, props.width);
  renderer.pop_mask();
}

bool TextInputSystem::mouse_event(Element& e, const MouseEvent& event) {
  const auto& props = *e.props.cast<TextInputProps>();

  Vecf text_origin =
      e.position + props.border_width.offset() + props.padding.offset();

  const auto& font =
      fm->font_structure(props.text_style.font, props.text_style.font_size);

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
          "[TextInputSystem::key_event] Enter changed: %s -> %s",
          props.text.c_str(),
          active_text.c_str());
      props.text = active_text;
      props.changed = true;
      return true;
    }
    DATAGUI_LOG("[TextInputSystem::key_event] Enter unchanged");
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
