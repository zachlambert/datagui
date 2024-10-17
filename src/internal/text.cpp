#include "datagui/internal/text.hpp"
#include <string>
#include <array>
#include <filesystem>
#include <unordered_map>
#include <stdexcept>
#include <algorithm>
#include <GL/glew.h>
#include <optional>
#include <iostream> // TEMP
#include "datagui/internal/shader.hpp"
#include "datagui/exception.hpp"


extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
}

namespace datagui {

const static std::string vertex_shader = R"(
#version 330 core

// Input vertex data: position and normal
layout(location = 0) in vec3 vertex_pos;
layout(location = 1) in vec2 uv;

out vec2 fs_uv;

void main(){
    gl_Position = vec4(vertex_pos.x, vertex_pos.y, 0, 1);
    fs_uv = uv;
}
)";

const static std::string fragment_shader = R"(
#version 330 core

in vec2 fs_uv;

uniform sampler2D char_texture;
out vec4 color;

void main(){
    color = vec4(1, 1, 1, texture(tex, fs_uv).x);
}
)";

std::string find_font_path(Font font) {
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
        throw InitializationError("Failed to find font");
    }

    std::sort(candidates.begin(), candidates.end(),
        // Return true if a.stem < b.stem
        [](const std::filesystem::path& a, const std::filesystem::path& b) -> bool {
            return a.stem().string().size() < b.stem().string().size();
        }
    );
    return candidates.front();
}

FontStructure load_font(Font font, int font_size) {
    FontStructure structure;
    structure.resize(' ', '~');

    // Initialise ft_library

    FT_Library ft_library;
    if (FT_Init_FreeType(&ft_library) != 0) {
        throw std::runtime_error("Failed to initialize freetype library");
    }

    FT_Face ft_face;
    if (FT_New_Face(ft_library, find_font_path(font).c_str(), 0, &ft_face) != 0) {
        throw InitializationError("Failed to load font");
    }

    FT_Set_Pixel_Sizes(ft_face, 0, font_size);
    structure.ascender = float(ft_face->ascender) / 128;
    structure.descender = -float(ft_face->descender) / 128;
    structure.line_height = float(ft_face->height) / 128;

    // 1st pass iterate through characters
    // -> Read properties and find required texture height

    const std::size_t texture_width = 256;
    std::size_t texture_height = structure.line_height;

    float texture_row_width = 0;
    for (int i = structure.char_first(); i < structure.char_end(); i++) {
        if (FT_Load_Char(ft_face, char(i), FT_LOAD_RENDER) != 0) {
            throw InitializationError("Failed to load character: " + std::to_string(char(i)));
        }

        auto& character = structure.get(i);
        character.size = Vecf(
            ft_face->glyph->bitmap.width,
            ft_face->glyph->bitmap.rows);
        character.offset = Vecf(
            ft_face->glyph->bitmap_left,
            float(ft_face->glyph->bitmap_top) - ft_face->glyph->bitmap.rows);
        character.advance = float(ft_face->glyph->advance.x) / 64;

        if (texture_row_width + character.advance > texture_width) {
            texture_height += structure.line_height;
            texture_row_width = 0;
        }
        texture_row_width += character.advance;
    }

    // Load shader program and buffers

    struct Vertex {
        Vecf pos;
        Vecf uv;
    };

    int shader_program = create_program(vertex_shader, fragment_shader);

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

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

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create font texture

    glGenTextures(1, &structure.font_texture);
    glBindTexture(GL_TEXTURE_2D, structure.font_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Bind the font to a framebuffer to draw to

    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, structure.font_texture, 0);

    // 2nd pass iterate through characters
    // -> Render to the font texture

    glUseProgram(shader_program);
    glViewport(0, 0, texture_width, texture_height);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
    glBindVertexArray(VAO);

    float char_x = 0;
    float char_y = 0;
    for (int i = structure.char_first(); i < structure.char_end(); i++) {
        if (FT_Load_Char(ft_face, char(i), FT_LOAD_RENDER) != 0) {
            throw std::runtime_error("Failed to load character: " + std::to_string(char(i)));
        }

        auto& character = structure.get(i);

        // Calculate where the character should be drawn on the texture

        if (char_x + character.advance > texture_width) {
            char_x = 0;
            char_y += structure.line_height;
        }

        Vecf bottom_left(
            (char_x + character.offset.x) / texture_width,
            (char_y + structure.descender + character.offset.y) / texture_height
        );
        Vecf top_right(
            bottom_left.x + ft_face->glyph->bitmap.width,
            bottom_left.y + ft_face->glyph->bitmap.rows
        );
        Vecf bottom_right(top_right.x, bottom_left.y);
        Vecf top_left(bottom_left.x, top_right.y);

        character.uv = Boxf(bottom_left, top_right);

        char_x += character.advance;

        // Load the vertices

        std::vector<Vertex> vertices = {
            Vertex{bottom_left, Vecf(0, 0)},
            Vertex{bottom_right, Vecf(1, 0)},
            Vertex{top_left, Vecf(0, 1)},
            Vertex{bottom_right, Vecf(1, 0)},
            Vertex{top_right, Vecf(1, 1)},
            Vertex{top_left, Vecf(0, 1)}
        };

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Vertex),
            vertices.data(),
            GL_STATIC_DRAW
        );
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Generate a texture for the character bitmap

        unsigned int char_texture;
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

        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        glBindTexture(GL_TEXTURE0, 0);
        glDeleteTextures(1, &char_texture);
    }

    // Cleanup

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteProgram(shader_program);
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);

    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_library);

    return structure;
}

Vecf text_size(const FontStructure& font, const std::string& text, float max_width) {
    bool has_max_width = max_width > 0;
    Vecf pos = Vecf::Zero();
    pos.y += font.line_height;

    for (char c: text) {
        if (!font.char_valid(c)) {
            continue;
        }
        const auto& character = font.get(c);
        if (has_max_width && pos.x + character.advance > max_width) {
            pos.x = 0;
            pos.y += font.line_height;
        }
        pos.x += character.advance;
    }

    if (has_max_width) {
        return Vecf(max_width, pos.y);
    } else {
        return pos;
    }
}

std::size_t find_cursor(
    const FontStructure& font,
    const std::string& text,
    float max_width,
    const Vecf& point)
{
    bool has_max_width = max_width > 0;
    Vecf pos = Vecf::Zero();
    pos.y += font.line_height;

    std::size_t column = 0;
    bool column_found = false;

    for (std::size_t i = 0; i < text.size(); i++) {
        if (!font.char_valid(text[i])) {
            continue;
        }
        const auto& c = font.get(text[i]);

        if (!column_found && pos.x + c.advance/2 > point.x) {
            column_found = true;
            column = i;
            if (point.y < pos.y) {
                return column;
            }
        }

        if (has_max_width && pos.x + c.advance > max_width) {
            if (!column_found) {
                column = i+1;
            }
            if (point.y < pos.y) {
                return column;
            }
            pos.x = 0;
            pos.y += font.line_height;
        }
        pos.x += c.advance;
    }
    return column;
}

Vecf cursor_offset(
    const FontStructure& font,
    const std::string& text,
    float max_width,
    std::size_t cursor)
{
    bool has_max_width = max_width > 0;
    Vecf offset = Vecf::Zero();
    offset.y += font.line_height;

    for (std::size_t i = 0; i < cursor; i++) {
        if (!font.char_valid(text[i])) {
            continue;
        }
        const auto& c = font.get(text[i]);
        if (has_max_width && offset.x + c.advance > max_width) {
            offset.x = 0;
            offset.y += font.line_height;
        }
        offset.x += c.advance;
    }
    return offset;
}

} // namespace datagui
