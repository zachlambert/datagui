#pragma once

#include "datagui/internal/text.hpp"
#include "datagui/internal/geometry_renderer.hpp"

namespace datagui {

struct TextSelection {
    std::size_t begin;
    std::size_t end;

    TextSelection():
        begin(0),
        end(0)
    {}

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

void selection_input_key(
    std::string& text,
    TextSelection& selection,
    int key,
    int mods,
    bool editable);

void selection_input_char(
    std::string& text,
    TextSelection& selection,
    char character,
    bool editable);

void render_selection(
    const Style& style,
    const FontStructure& font,
    const std::string& text,
    float max_width,
    const Vecf& origin,
    const TextSelection& selection,
    bool editable,
    GeometryRenderer& geometry_renderer);

} // namespace datagui
