#pragma once

#include <vector>
#include <string>
#include "datagui/geometry.hpp"
#include "datagui/color.hpp"
#include "datagui/internal/text.hpp"


namespace datagui {

class TextRenderer {
public:
    void init();

    void queue_text(
        const FontStructure& font,
        const Color& font_color,
        const std::string& text,
        float max_width,
        const Vecf& origin,
        float depth);

    void render(const Vecf& viewport_size);

private:
    struct Vertex {
        Vecf pos;
        Vecf uv;
        float depth;
    };

    struct Command {
        const unsigned int font_texture;
        const Color font_color;
        std::vector<Vertex> vertices;

        Command(unsigned int font_texture, const Color& font_color):
            font_texture(font_texture), font_color(font_color)
        {}
    };

    std::vector<Command> commands;

    struct {
        // Shader
        unsigned int program_id;
        // Uniforms
        unsigned int uniform_viewport_size;
        unsigned int uniform_text_color;
        // Array/buffer objects
        unsigned int VAO, VBO;
    } gl_data;
};

} // namespace datagui
