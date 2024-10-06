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

    void queue_text(int depth, const Vecf& origin, const std::string& text, float line_width);

    void render(const Vecf& viewport_size);

private:
    struct Vertex {
        Vecf pos;
        Vecf uv;
    };

    struct Character {
        Boxf uv;
        Vecf size;
        Vecf offset;
        int advance;
    };
    std::vector<Character> characters;

    struct Command {
        int depth;
        Vecf origin;
        std::string text;
        Color color;
        int line_width;
    };
    std::vector<Command> commands;

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
