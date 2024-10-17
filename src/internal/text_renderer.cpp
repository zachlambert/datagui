#include "datagui/internal/text_renderer.hpp"
#include <string>
#include <array>
#include <filesystem>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include <GL/glew.h>
#include <optional>
#include "datagui/internal/shader.hpp"


extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
}

namespace datagui {

const static std::string texture_vs = R"(
#version 330 core

// Input vertex data: position and normal
layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in float depth;

uniform vec2 viewport_size;
out vec2 fs_uv;

void main(){
    gl_Position = vec4(
        -1.f + 2 * vertex_pos.x / viewport_size.x,
        1.f - 2 * vertex_pos.y / viewport_size.y,
        depth,
        1
    );
    fs_uv = uv;
}
)";

const static std::string texture_fs = R"(
#version 330 core

in vec2 fs_uv;

uniform sampler2D tex;
uniform vec4 text_color;

out vec4 color;

void main(){
    color = vec4(text_color.xyz, texture(tex, fs_uv).x * text_color[3]);
}
)";

std::optional<std::string> find_font_path(Font font) {
    static const std::unordered_map<Font, std::string> names = {
        { Font::DejaVuSans, "DejaVuSans" },
        { Font::DejaVuSerif, "DejaVuSerif" },
        { Font::DejaVuSansMono, "DejaVuSansMono" }
    };
    // Crash if the above list is missing an entry for a given font
    std::string font_name = names.at(font);

    std::vector<std::filesystem::path> candidates;

    std::vector<std::string> paths = {
        "/usr/share/fonts"
    };

    for (const auto& path: paths) {
        for (
            auto iter = std::filesystem::recursive_directory_iterator(path);
            iter != std::filesystem::recursive_directory_iterator();
            ++iter)
        {
            if (iter->is_directory()) continue;
            if (iter->path().extension() != ".ttf") continue;

            std::string name = iter->path().stem();
            if (name.find(font_name) == std::string::npos) continue;

            candidates.push_back(iter->path());
        }
    }

    if (candidates.empty()) {
        return std::nullopt;
    }

    std::sort(candidates.begin(), candidates.end(),
        // Return true if a.stem < b.stem
        [](const std::filesystem::path& a, const std::filesystem::path& b) -> bool {
            return a.stem().string().size() < b.stem().string().size();
        }
    );
    return candidates.front();
}

TextRenderer::TextRenderer():
    ascender(0),
    descender(0)
{}

void TextRenderer::init(const Style::Text& style) {
    this->style = style;

    // Configure shader program and buffers

    gl_data.program_id = create_program(texture_vs, texture_fs);
    gl_data.uniform_viewport_size = glGetUniformLocation(gl_data.program_id, "viewport_size");
    gl_data.uniform_text_color = glGetUniformLocation(gl_data.program_id, "text_color");

    glGenVertexArrays(1, &gl_data.VAO);
    glGenBuffers(1, &gl_data.VBO);

    glBindVertexArray(gl_data.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, gl_data.VBO);

    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, pos)
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, uv)
    );
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        2, 1, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, depth)
    );
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create font texture

    int font_tex_width = 1024; // TODO: Better method to choose these sizes
    int font_tex_height = 728;
    glGenTextures(1, &gl_data.font_texture);
    glBindTexture(GL_TEXTURE_2D, gl_data.font_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, font_tex_width, font_tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Draw to the font texture with a temporary frame buffer

    unsigned int temp_framebuffer;
    glGenFramebuffers(1, &temp_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, temp_framebuffer);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, gl_data.font_texture, 0);

    draw_font_bitmap(font_tex_width, font_tex_height, style.font, style.font_size);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &temp_framebuffer);
}

void TextRenderer::queue_text(
    const std::string& text,
    float max_width,
    const Vecf& origin,
    float depth,
    const Color& text_color)
{
    bool has_max_width = max_width > 0;
    Vecf offset = Vecf::Zero();
    offset.y += line_height;

    for (char c_char: text) {
        int c_index = int(c_char) - char_first;
        if (c_index < 0 || c_index >= characters.size()) {
            continue;
        }
        const Character& c = characters[c_index];
        if (has_max_width && offset.x + c.advance > max_width) {
            offset.x = 0;
            offset.y += line_height;
        }

        Vecf pos = origin + offset + Vecf(0, -descender) + c.offset;

        Boxf box(pos, pos + c.size);
        const Boxf& uv = c.uv;

        vertices.push_back(Vertex{box.bottom_left(), uv.top_left(), depth});
        vertices.push_back(Vertex{box.bottom_right(), uv.top_right(), depth});
        vertices.push_back(Vertex{box.top_left(), uv.bottom_left(), depth});
        vertices.push_back(Vertex{box.bottom_right(), uv.top_right(), depth});
        vertices.push_back(Vertex{box.top_right(), uv.bottom_right(), depth});
        vertices.push_back(Vertex{box.top_left(), uv.bottom_left(), depth});

        offset.x += c.advance;
    }
}

void TextRenderer::render(const Vecf& viewport_size) {
    glBindVertexArray(gl_data.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, gl_data.VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_STATIC_DRAW
    );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // TODO: Use user-defined color
    Color text_color = Color::Black();

    glUseProgram(gl_data.program_id);
    glUniform4f(gl_data.uniform_text_color, text_color.r, text_color.g, text_color.b, text_color.a);
    glUniform2f(gl_data.uniform_viewport_size, viewport_size.x, viewport_size.y);
    glBindTexture(GL_TEXTURE_2D, gl_data.font_texture);

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    vertices.clear();
}

void TextRenderer::draw_font_bitmap(
    int width,
    int height,
    Font font,
    int font_size)
{
    glViewport(0, 0, width, height);

    FT_Library ft_library;
    if (FT_Init_FreeType(&ft_library) != 0) {
        throw std::runtime_error("Failed to initialize freetype library");
    }

    FT_Face ft_face;
    if (auto path = find_font_path(font); path.has_value()) {
        if (FT_New_Face(ft_library, path.value().c_str(), 0, &ft_face) != 0) {
            throw std::runtime_error("Failed to load font");
        }
    } else {
        throw std::runtime_error("Failed to find font");
    }
    FT_Set_Pixel_Sizes(ft_face, 0, font_size);
    ascender = float(ft_face->ascender) / 128;
    descender = -float(ft_face->descender) / 128;
    line_height = float(ft_face->height) / 128;

    int font_pos_x = 0;
    int font_pos_y = 0;
    int font_row_height = 0;
    for (int i = char_first; i <= char_last; i++) {
        if (FT_Load_Char(ft_face, char(i), FT_LOAD_RENDER) != 0) {
            throw std::runtime_error("Failed to load character: " + std::to_string(char(i)));
        }

        if (font_pos_x + ft_face->glyph->bitmap.width > width) {
            font_pos_x = 0;
            font_pos_y += font_row_height;
            font_row_height = 0;
        }

        unsigned int char_texture;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
        glGenTextures(1, &char_texture);
        glBindTexture(GL_TEXTURE_2D, char_texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            ft_face->glyph->bitmap.width,
            ft_face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            ft_face->glyph->bitmap.buffer
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Vecf bottom_left(
            font_pos_x,
            font_pos_y+ft_face->glyph->bitmap.rows);
        Vecf top_right(
            font_pos_x + ft_face->glyph->bitmap.width,
            font_pos_y);
        Vecf bottom_right(top_right.x, bottom_left.y);
        Vecf top_left(bottom_left.x, top_right.y);

        std::vector<Vertex> vertices = {
            Vertex{bottom_left, Vecf(0, 1), 0},
            Vertex{bottom_right, Vecf(1, 1), 0},
            Vertex{top_left, Vecf(0, 0), 0},
            Vertex{bottom_right, Vecf(1, 1), 0},
            Vertex{top_right, Vecf(1, 0), 0},
            Vertex{top_left, Vecf(0, 0), 0}
        };

        glBindBuffer(GL_ARRAY_BUFFER, gl_data.VBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Vertex),
            vertices.data(),
            GL_STATIC_DRAW
        );
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glUseProgram(gl_data.program_id);
        glUniform2f(gl_data.uniform_viewport_size, width, height);
        glUniform4f(gl_data.uniform_text_color, 1, 1, 1, 1);
        glBindVertexArray(gl_data.VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        glBindTexture(GL_TEXTURE0, 0);
        glDeleteTextures(1, &char_texture);

        font_row_height = std::max(font_row_height, int(ft_face->glyph->bitmap.rows));
        font_pos_x += ft_face->glyph->bitmap.width;

        Character character;
        character.uv = Boxf(
            Vecf(bottom_left.x / width, 1 - bottom_left.y / height),
            Vecf(top_right.x / width, 1 - top_right.y / height)
        );

        character.size = Vecf(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows);
        character.offset = Vecf(ft_face->glyph->bitmap_left, -float(ft_face->glyph->bitmap_top));
        character.advance = float(ft_face->glyph->advance.x) / 64;

        characters.push_back(character);
    }

    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_library);
}

Vecf TextRenderer::text_size(
    const std::string& text,
    float max_width) const
{
    bool has_max_width = max_width > 0;
    Vecf offset = Vecf::Zero();
    offset.y += line_height;

    for (char c_char: text) {
        int c_index = int(c_char) - char_first;
        if (c_index < 0 || c_index >= characters.size()) {
            continue;
        }
        const Character& c = characters[c_index];
        if (has_max_width && offset.x + c.advance > max_width) {
            offset.x = 0;
            offset.y += line_height;
        }
        offset.x += c.advance;
    }

    if (has_max_width) {
        return Vecf(max_width, offset.y);
    } else {
        return offset;
    }
}

TextStructure TextRenderer::calculate_text_structure(
    const std::string& text,
    float width) const
{
    TextStructure structure;
    structure.width = width;
    structure.line_height = line_height;

    float line_width = 0;
    std::size_t line_begin = 0;

    for (std::size_t i = 0; i < text.size(); i++) {
        int c_index = int(text[i]) - char_first;
        if (c_index < 0 || c_index >= characters.size()) {
            continue;
        }
        const Character& c = characters[c_index];
        if (line_width + c.advance > width) {
            std::size_t line_end = i;
            structure.lines.push_back(LineStructure{line_begin, line_end, line_width});
            line_begin = line_end;
            line_width = 0;
        }
        line_width += c.advance;
    }
    structure.lines.push_back(LineStructure{line_begin, text.size(), line_width});

    return structure;
}

CursorPos TextRenderer::find_cursor(
    const std::string& text,
    const TextStructure& structure,
    const Vecf& point) const
{
    if (structure.lines.empty()) {
        return CursorPos{0, Vecf::Zero()};
    }

    float y = 0;
    for (std::size_t line_i = 0; line_i < structure.lines.size(); line_i++) {
        const auto& line = structure.lines[line_i];
        if (point.y >= y + line_height && line_i != structure.lines.size()-1) {
            y += line_height;
            continue;
        }

        float x = 0;
        for (std::size_t char_i = line.begin; char_i < line.end; char_i++) {
            int c_index = int(text[char_i]) - char_first;
            if (c_index < 0 || c_index >= characters.size()) {
                continue;
            }
            const Character& c = characters[c_index];
            if (point.x < x + float(c.advance)/2) {
                return CursorPos{char_i, Vecf(x, y)};
            }
            x += c.advance;
        }
        return CursorPos{line.end, Vecf(x, y)};
    }
    // Unreachable
    throw std::runtime_error("Unreachable code reached");
    return CursorPos{0, Vecf::Zero()};
}

Vecf TextRenderer::find_cursor_offset(
    const std::string& text,
    const TextStructure& structure,
    std::size_t index) const
{
    if (structure.lines.empty()) {
        return Vecf::Zero();
    }

    if (index > text.size()) {
        throw std::runtime_error("Invalid cursor index");
    }

    for (std::size_t line_i = 0; line_i < structure.lines.size(); line_i++) {
        const auto& line = structure.lines[line_i];
        if (index >= line.end && line_i != structure.lines.size()-1) {
            continue;
        }
        float x = 0;
        for (std::size_t i = line.begin; i < index; i++) {
            int c_index = int(text[i]) - char_first;
            if (c_index < 0 || c_index >= characters.size()) {
                continue;
            }
            x += characters[c_index].advance;
        }
        return Vecf{x, line_i * line_height};
    }
    throw std::runtime_error("Invalid cursor");
    return Vecf::Zero();
}

} // namespace datagui
