#pragma once

#include <vector>
#include <string>
#include "datagui/geometry.hpp"
#include "datagui/color.hpp"
#include "datagui/style.hpp"


namespace datagui {

struct LineStructure {
    std::size_t begin;
    std::size_t end;
    float width;
};

struct TextStructure {
    float width;
    float line_height;
    std::vector<LineStructure> lines;
};

struct CursorPos {
    std::size_t index;
    Vecf offset;
};

class TextRenderer {
public:
    TextRenderer();
    void init(const Style::Text& style);

    void queue_text(
        const std::string& text,
        float max_width,
        const Vecf& origin,
        float depth,
        const Color& text_color);

    void render(const Vecf& viewport_size);

    Vecf text_size(
        const std::string& text,
        float max_width) const;

    TextStructure calculate_text_structure(
        const std::string& text,
        float width) const;

    CursorPos find_cursor(
        const std::string& text,
        const TextStructure& structure,
        const Vecf& point) const;

    Vecf find_cursor_offset(
        const std::string& text,
        const TextStructure& structure,
        std::size_t index) const;

private:
    void draw_font_bitmap(int width, int height, Font font, int font_size);

    Style::Text style;

    struct Character {
        Boxf uv;
        Vecf size;
        Vecf offset;
        int advance;
    };
    std::vector<Character> characters;
    float line_height;
    float ascender;
    float descender;

    struct Vertex {
        Vecf pos;
        Vecf uv;
        float depth;
    };
    std::vector<Vertex> vertices;

    struct {
        // Shader
        unsigned int program_id;
        // Framebuffer for font
        unsigned int font_texture;
        // Uniforms
        unsigned int uniform_viewport_size;
        unsigned int uniform_text_color;
        // Array/buffer objects
        unsigned int VAO, VBO;
    } gl_data;

    static const int char_first = int(' ');
    static const int char_last = int('~');
};

} // namespace datagui
