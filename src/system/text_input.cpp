#include "datagui/system/text_input.hpp"

namespace datagui {

void TextInputSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& text_input = element.text_input();

  const std::string& text = state.focused ? active_text : text_input.text;
  state.fixed_size =
      2.f * Vecf::Constant(theme->input_border_width + theme->text_padding);
  state.dynamic_size = Vecf::Zero();
  state.floating = 0;

  Vecf text_size =
      fm->text_size(text, theme->text_font, theme->text_size, text_input.width);
  state.fixed_size.y += text_size.y;

  if (auto width = std::get_if<LengthFixed>(&text_input.width)) {
    state.fixed_size.x += width->value;
  } else {
    state.fixed_size.x += text_size.x;
    if (auto width = std::get_if<LengthDynamic>(&text_input.width)) {
      state.dynamic_size.x = width->weight;
    }
  }
}

void TextInputSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& text_input = element.text_input();

  const std::string& text = state.focused ? active_text : text_input.text;

  Color border_color;
  if (state.in_focus_tree) {
    border_color = theme->input_color_border_focus;
  } else {
    border_color = theme->input_color_border;
  }

  renderer.queue_box(
      state.box(),
      theme->input_color_bg,
      theme->input_border_width,
      border_color,
      0);

  Vecf text_position =
      state.position +
      Vecf::Constant(theme->input_border_width + theme->text_padding);

  if (state.focused) {
    render_selection(
        text,
        text_position,
        active_selection,
        fm->font_structure(theme->text_font, theme->text_size),
        theme->text_cursor_color,
        theme->text_highlight_color,
        theme->text_cursor_width,
        text_input.width,
        renderer);
  }

  Boxf mask;
  mask.lower = state.position +
               Vecf::Constant(theme->input_border_width + theme->text_padding);
  mask.upper = state.position + state.size -
               Vecf::Constant(theme->input_border_width + theme->text_padding);

  renderer.push_mask(mask);
  renderer.queue_text(
      text_position,
      text,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      text_input.width);
  renderer.pop_mask();
}

bool TextInputSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  const auto& text_input = element.text_input();

  Vecf text_origin =
      state.position +
      Vecf::Constant(theme->input_border_width + theme->text_padding);

  const auto& font = fm->font_structure(theme->text_font, theme->text_size);

  if (event.action == MouseAction::Press) {
    active_text = text_input.text;
  }

  std::size_t cursor_pos = find_cursor(
      font,
      active_text,
      text_input.width,
      event.position - text_origin);

  if (event.action == MouseAction::Press) {
    active_selection.reset(cursor_pos);
  } else if (event.action == MouseAction::Hold) {
    active_selection.end = cursor_pos;
  }

  return false;
}

bool TextInputSystem::key_event(ElementPtr element, const KeyEvent& event) {
  auto& text_input = element.text_input();

  if (event.action == KeyAction::Press && event.key == Key::Enter) {
    if (text_input.text != active_text) {
      text_input.text = active_text;
      if (!text_input.callback) {
        return true;
      }
      text_input.callback(text_input.text);
    }
    return false;
  }

  selection_key_event(active_text, active_selection, true, event);
  return false;
}

bool TextInputSystem::text_event(ElementPtr element, const TextEvent& event) {
  selection_text_event(active_text, active_selection, true, event);
  return false;
}

void TextInputSystem::focus_enter(ElementPtr element) {
  const auto& props = element.text_input();

  active_selection.reset(0);
  active_text = props.text;
}

bool TextInputSystem::focus_leave(ElementPtr element, bool success) {
  auto& text_input = element.text_input();
  if (success && text_input.text != active_text) {
    text_input.text = active_text;
    if (!text_input.callback) {
      return true;
    }
    text_input.callback(text_input.text);
  }
  return false;
}

} // namespace datagui
