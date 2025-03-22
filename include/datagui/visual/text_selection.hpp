#pragma once

#include "datagui/visual/font.hpp"
#include "datagui/visual/geometry_renderer.hpp"

namespace datagui {

struct TextSelection {
  std::size_t begin;
  std::size_t end;

  TextSelection() : begin(0), end(0) {}

  void reset(std::size_t pos) {
    begin = pos;
    end = pos;
  }
  std::size_t span() const {
    return end >= begin ? (end - begin) : (begin - end);
  }
  std::size_t from() const {
    return std::min(begin, end);
  }
  std::size_t to() const {
    return std::max(begin, end);
  }
};

enum class KeyValue { Backspace, LeftArrow, RightArrow, Enter };

struct KeyEvent {
  bool is_text; // else is_char
  KeyValue key_value;
  bool key_release; // else press
  bool key_shift;
  bool key_ctrl;
  char text_value;

  static KeyEvent key(KeyValue key, bool release, bool shift, bool ctrl) {
    KeyEvent event;
    event.is_text = false;
    event.key_value = key;
    event.key_release = release;
    event.key_shift = shift;
    event.key_ctrl = ctrl;
    return event;
  }
  static KeyEvent text(char text) {
    KeyEvent event;
    event.is_text = true;
    event.text_value = text;
    return event;
  }
};

std::size_t find_cursor(
    const FontStructure& font,
    const std::string& text,
    float max_width,
    const Vecf& point);

Vecf cursor_offset(
    const FontStructure& font,
    const std::string& text,
    float max_width,
    std::size_t cursor);

void selection_key_event(
    std::string& text,
    TextSelection& selection,
    bool editable,
    const KeyEvent& event);

struct TextSelectionStyle {
  float cursor_width = 2;
  Color cursor_color = Color::Gray(0.5);
  Color highlight_color = Color(0.3, 0.8, 1.0);
};

void render_selection(
    const TextSelectionStyle& style,
    const FontStructure& font,
    const std::string& text,
    float max_width,
    const Vecf& origin,
    const TextSelection& selection,
    bool editable,
    GeometryRenderer& geometry_renderer);

} // namespace datagui
