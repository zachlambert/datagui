#pragma once

#include <vector>
#include <string>
#include "datagui/geometry.hpp"
#include "datagui/color.hpp"
#include "datagui/font.hpp"

namespace datagui {

class TextRenderer {
public:
    TextRenderer();
    void init(Font font, int font_size);

    void queue_text(const std::string& text, float max_width, float line_height_factor, const Vecf& origin, float depth, const Color& text_color);
    Vecf text_size(const std::string& text, float max_width, float line_height_factor);

    void render(const Vecf& viewport_size);
    int get_font_size() const { return font_size; }

private:
    void draw_font_bitmap(int width, int height, Font font, int font_size);

    int font_size;

    struct Character {
        Boxf uv;
        Vecf size;
        Vecf offset;
        int advance;
    };
    std::vector<Character> characters;

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
};

} // namespace datagui
