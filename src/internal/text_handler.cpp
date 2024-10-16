#include "datagui/internal/text_handler.hpp"
#include <GLFW/glfw3.h>


namespace datagui {

TextHandler::TextHandler(const TextRenderer& text_renderer, const Style& style):
    text_renderer(text_renderer),
    style(style)
{}

void TextHandler::select(
    const std::string& text,
    float width,
    float line_height,
    const Vecf& point)
{
    initial_text = text;
    structure = text_renderer.calculate_text_structure(text, width, line_height);
    cursor_begin = text_renderer.find_cursor(text, structure, point);
    cursor_end = cursor_begin;
}

void TextHandler::select_index(
    const std::string& text,
    float width,
    float line_height,
    std::size_t index)
{
    initial_text = text;
    structure = text_renderer.calculate_text_structure(text, width, line_height);
    cursor_begin.index = index;
    cursor_begin.offset = text_renderer.find_cursor_offset(text, structure, index);
    cursor_end = cursor_begin;
}

void TextHandler::drag(const std::string& text, const Vecf& point) {
    cursor_end = text_renderer.find_cursor(
        text,
        structure,
        point
    );
}

void TextHandler::revert(std::string& text) {
    text = initial_text;
}

void TextHandler::input_key(std::string& text, int key, int mods, bool editable) {
    switch (key) {
        case GLFW_KEY_LEFT:
        {
            if (cursor_begin.index != cursor_end.index && mods != 1) {
                if (cursor_begin.index <= cursor_end.index) {
                    cursor_end = cursor_begin;
                } else {
                    cursor_begin = cursor_end;
                }
            } else if (cursor_end.index != 0) {
                cursor_end.index--;
                cursor_end.offset = text_renderer.find_cursor_offset(
                    text, structure, cursor_end.index);
                if (mods != 1) {
                    cursor_begin = cursor_end;
                }
            }
            break;
        }
        case GLFW_KEY_RIGHT:
        {
            if (cursor_begin.index != cursor_end.index && mods != 1) {
                if (cursor_begin.index <= cursor_end.index) {
                    cursor_begin = cursor_end;
                } else {
                    cursor_end = cursor_begin;
                }
            } else if (cursor_end.index != text.size()) {
                cursor_end.index++;
                cursor_end.offset = text_renderer.find_cursor_offset(
                    text, structure, cursor_end.index);
                if (mods != 1) {
                    cursor_begin = cursor_end;
                }
            }
            break;
        }
        case GLFW_KEY_BACKSPACE:
        {
            if (!editable) {
                break;
            }
            if (cursor_begin.index != cursor_end.index) {
                std::size_t from = cursor_begin.index;
                std::size_t to = cursor_end.index;
                if (from > to) {
                    std::swap(from, to);
                }
                text.erase(text.begin() + from, text.begin() + to);
                if (cursor_begin.index <= cursor_end.index) {
                    cursor_end = cursor_begin;
                } else {
                    cursor_begin = cursor_end;
                }
                structure = text_renderer.calculate_text_structure(
                    text,
                    structure.width,
                    style.text.line_height);

            } else if (cursor_begin.index > 0) {
                text.erase(text.begin() + (cursor_begin.index-1));
                structure = text_renderer.calculate_text_structure(
                    text,
                    structure.width,
                    style.text.line_height);
                cursor_begin.index--;
                cursor_begin.offset = text_renderer.find_cursor_offset(
                    text, structure, cursor_begin.index);
                cursor_end = cursor_begin;
            }
            break;
        }
    }
}

void TextHandler::input_char(std::string& text, char character) {
    if (cursor_begin.index != cursor_end.index) {
        std::size_t from = cursor_begin.index;
        std::size_t to = cursor_end.index;
        if (from > to) {
            std::swap(from, to);
        }
        text.erase(text.begin() + from, text.begin() + to);
        cursor_begin.index = from;
    }

    text.insert(text.begin() + cursor_begin.index, character);
    cursor_begin.index++;
    structure = text_renderer.calculate_text_structure(
        text,
        structure.width,
        style.text.line_height);
    cursor_begin.offset = text_renderer.find_cursor_offset(
        text, structure, cursor_begin.index);
    cursor_end = cursor_begin;
}

void TextHandler::render(
    const std::string& text,
    const Vecf& origin,
    bool editable,
    float normalized_depth,
    GeometryRenderer& geometry_renderer)
{
    // Render cursor

    if (cursor_begin.index == cursor_end.index) {
        if (!editable) {
            return;
        }
        geometry_renderer.queue_box(
            normalized_depth,
            Boxf(
                origin+cursor_begin.offset-Vecf(float(style.text_input.cursor_width)/2, 0),
                origin+cursor_begin.offset + Vecf(style.text_input.cursor_width, structure.line_height)),
            style.text_input.cursor_color,
            0,
            Color::Black(),
            0
        );
        return;
    }

    // Render higlighlight

    CursorPos from, to;
    if (cursor_begin.index <= cursor_end.index) {
        from = cursor_begin;
        to = cursor_end;
    } else {
        from = cursor_end;
        to = cursor_begin;
    }

    for (std::size_t line_i = 0; line_i < structure.lines.size(); line_i++) {
        const auto& line = structure.lines[line_i];
        if (from.index >= line.end) {
            continue;
        }
        CursorPos line_to;
        if (to.index > line.end) {
            line_to = CursorPos{line.end, Vecf(line.width, line_i * structure.line_height)};
        } else {
            line_to = to;
        }
        geometry_renderer.queue_box(
            normalized_depth,
            Boxf(
                origin + from.offset,
                origin + line_to.offset + Vecf(0, structure.line_height)
            ),
            style.text_input.highlight_color,
            0,
            Color::Black(),
            0
        );
        if (line_i != structure.lines.size()-1) {
            const auto& next_line = structure.lines[line_i+1];
            from.index = next_line.begin;
            from.offset = Vecf(0, (line_i+1) * structure.line_height);
        }
        if (to.index <= line.end) {
            break;
        }
    }
}

} // namespace datagui
