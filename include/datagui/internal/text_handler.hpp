#pragma once

#include "datagui/internal/text_renderer.hpp"
#include "datagui/internal/geometry_renderer.hpp"

namespace datagui {

class TextHandler {
public:
    TextHandler(const TextRenderer& renderer, const Style& style);

    void select(
        const std::string& text,
        float width,
        const Vecf& point);

    void select_index(
        const std::string& text,
        float width,
        std::size_t index);

    void drag(
        const std::string& text,
        const Vecf& point);

    void revert(std::string& text);

    void input_key(std::string& text, int key, int mods, bool editable);
    void input_char(std::string& text, char character);

    void render(
        const std::string& text,
        const Vecf& origin,
        bool editable,
        float normalized_depth,
        GeometryRenderer& renderer);

private:
    const TextRenderer& text_renderer;
    const Style& style;

    std::string initial_text;
    TextStructure structure;
    CursorPos cursor_begin;
    CursorPos cursor_end;
};

} // namespace datagui
