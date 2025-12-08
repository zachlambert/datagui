#include "datagui/system/text_input.hpp"

namespace datagui {

void TextInputSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& text_input = element.text_input();

  const std::string& text = state.focused ? active_text : text_input.text;
  Length text_length = text_input.width
                           ? *text_input.width
                           : LengthFixed(theme->text_input_default_width);
  Vec2 text_size =
      fm->text_size(text, theme->text_font, theme->text_size, text_length);

  state.fixed_size =
      text_size +
      2.f * Vec2::uniform(theme->input_border_width + theme->text_padding);
  state.dynamic_size = Vec2();
  state.floating = 0;

  if (text_input.width) {
    auto dynamic = std::get_if<LengthDynamic>(&(*text_input.width));
    if (dynamic) {
      state.dynamic_size.x = dynamic->weight;
    }
  }
}

void TextInputSystem::render(ConstElementPtr element, GuiRenderer& renderer) {
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
      border_color);

  Vec2 text_position =
      state.position +
      Vec2::uniform(theme->input_border_width + theme->text_padding);

  Length text_length = text_input.width
                           ? *text_input.width
                           : LengthFixed(theme->text_input_default_width);

  if (state.focused) {
    render_selection(
        text,
        text_position,
        active_selection,
        fm->font_structure(theme->text_font, theme->text_size),
        theme->text_cursor_color,
        theme->text_highlight_color,
        theme->text_cursor_width,
        text_length,
        renderer);
  }

  Box2 mask;
  mask.lower = state.position +
               Vec2::uniform(theme->input_border_width + theme->text_padding);
  mask.upper = state.position + state.size -
               Vec2::uniform(theme->input_border_width + theme->text_padding);

  renderer.push_mask(mask);
  renderer.queue_text(
      text_position,
      text,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      text_length);
  renderer.pop_mask();
}

void TextInputSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  const auto& text_input = element.text_input();

  if (event.action == MouseAction::Press) {
    active_text = text_input.text;
  }

  Vec2 text_origin =
      state.position +
      Vec2::uniform(theme->input_border_width + theme->text_padding);

  const auto& font = fm->font_structure(theme->text_font, theme->text_size);
  Length text_length = text_input.width
                           ? *text_input.width
                           : LengthFixed(theme->text_input_default_width);

  std::size_t cursor_pos =
      find_cursor(font, active_text, text_length, event.position - text_origin);

  if (event.action == MouseAction::Press) {
    active_selection.reset(cursor_pos);
  } else if (event.action == MouseAction::Hold) {
    active_selection.end = cursor_pos;
  }
}

void TextInputSystem::key_event(ElementPtr element, const KeyEvent& event) {
  auto& text_input = element.text_input();

  if (event.action == KeyAction::Press && event.key == Key::Enter) {
    if (text_input.text != active_text) {
      if (text_input.number_type &&
          !valid_text_to_number(*text_input.number_type, active_text)) {
        active_text = text_input.text;
        active_selection.reset(
            std::min(active_selection.begin, active_text.size()));
        return;
      }
      text_input.text = active_text;
      if (text_input.callback) {
        text_input.callback(text_input.text);
      } else {
        element.set_dirty();
      }
    }
    return;
  }

  selection_key_event(
      active_text,
      active_selection,
      text_input.editable,
      event);
}

void TextInputSystem::text_event(ElementPtr element, const TextEvent& event) {
  const auto& text_input = element.text_input();
  selection_text_event(
      active_text,
      active_selection,
      text_input.editable,
      event);
}

void TextInputSystem::focus_enter(ElementPtr element) {
  const auto& props = element.text_input();

  active_selection.reset(0);
  active_text = props.text;
}

void TextInputSystem::focus_leave(ElementPtr element, bool success) {
  auto& text_input = element.text_input();
  if (success && text_input.text != active_text) {
    if (text_input.number_type &&
        !valid_text_to_number(*text_input.number_type, active_text)) {
      return;
    }
    text_input.text = active_text;
    if (text_input.callback) {
      text_input.callback(text_input.text);
    } else {
      element.set_dirty();
    }
  }
}

} // namespace datagui
