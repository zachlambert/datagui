#pragma once

#include "datagui/input/event.hpp"
#include "datagui/style.hpp"
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

std::size_t find_cursor(
    const FontStructure& font,
    const std::string& text,
    Length text_width,
    const Vecf& point);

Vecf cursor_offset(
    const FontStructure& font,
    const std::string& text,
    Length text_width,
    std::size_t cursor);

void selection_key_event(
    std::string& text,
    TextSelection& selection,
    bool editable,
    const KeyEvent& event);

void selection_text_event(
    std::string& text,
    TextSelection& selection,
    bool editable,
    const TextEvent& event);

void render_selection(
    const FontStructure& font,
    const SelectableTextStyle& style,
    Length width,
    const std::string& text,
    const Vecf& origin,
    float z_pos,
    const TextSelection& selection,
    GeometryRenderer& geometry_renderer);

} // namespace datagui
