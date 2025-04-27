#include "datagui/visual/font.hpp"

#include "datagui/exception.hpp"
#include "datagui/visual/shader.hpp"
#include <GL/glew.h>
#include <algorithm>
#include <filesystem>
#include <string>
#include <unordered_map>

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
}

namespace datagui {

const static std::string vertex_shader = R"(
#version 330 core

// Input vertex data: position and normal
layout(location = 0) in vec2 vertex_pos;
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
  color = vec4(1, 1, 1, texture(char_texture, fs_uv).x);
}
)";

std::string find_font_path(Font font) {
  static const std::unordered_map<Font, std::string> names = {
      {Font::DejaVuSans, "DejaVuSans"},
      {Font::DejaVuSerif, "DejaVuSerif"},
      {Font::DejaVuSansMono, "DejaVuSansMono"}};
  // Crash if the above list is missing an entry for a given font
  std::string font_name = names.at(font);

  std::vector<std::filesystem::path> candidates;

  std::vector<std::string> paths = {"/usr/share/fonts"};

  for (const auto& path : paths) {
    for (auto iter = std::filesystem::recursive_directory_iterator(path);
         iter != std::filesystem::recursive_directory_iterator();
         ++iter) {
      if (iter->is_directory()) {
        continue;
      }
      if (iter->path().extension() != ".ttf") {
        continue;
      }

      std::string name = iter->path().stem();
      if (name.find(font_name) == std::string::npos) {
        continue;
      }

      candidates.push_back(iter->path());
    }
  }

  if (candidates.empty()) {
    throw InitializationError("Failed to find font");
  }

  std::sort(
      candidates.begin(),
      candidates.end(),
      // Return true if a.stem < b.stem
      [](const std::filesystem::path& a,
         const std::filesystem::path& b) -> bool {
        return a.stem().string().size() < b.stem().string().size();
      });
  return candidates.front();
}

FontStructure load_font(Font font, int font_size) {
  FontStructure structure;
  structure.resize(' ', '~');

  // Initialise ft_library

  FT_Library ft_library;
  if (FT_Init_FreeType(&ft_library) != 0) {
    throw InitializationError("Failed to initialize freetype library");
  }

  FT_Face ft_face;
  if (FT_New_Face(ft_library, find_font_path(font).c_str(), 0, &ft_face) != 0) {
    throw InitializationError("Failed to load font");
  }

  FT_Set_Pixel_Sizes(ft_face, 0, font_size);

  structure.ascender = float(ft_face->ascender) / 128;
  structure.descender = -float(ft_face->descender) / 128;
  structure.line_height = float(ft_face->height) / 128;

  // For some reason, the ascender, descender and line_height values aren't
  // scaled to the requested font size, even though the glyphs are
  float scale = float(font_size) / structure.line_height;
  structure.ascender *= scale;
  structure.descender *= scale;
  structure.line_height = font_size;

  // 1st pass iterate through characters
  // -> Read properties and find required texture height

  const std::size_t texture_width = 512;
  std::size_t texture_height = structure.line_height;

  float texture_row_width = 0;
  for (int i = structure.char_first(); i < structure.char_end(); i++) {
    if (FT_Load_Char(ft_face, char(i), 0) != 0) {
      throw InitializationError(
          "Failed to load character: " + std::to_string(char(i)));
    }

    auto& character = structure.get(i);
    character.size =
        Vecf(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows);
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
      0,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void*)offsetof(Vertex, pos));
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(
      1,
      2,
      GL_FLOAT,
      GL_FALSE,
      sizeof(Vertex),
      (void*)offsetof(Vertex, uv));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // Create font texture

  glGenTextures(1, &structure.font_texture);
  glBindTexture(GL_TEXTURE_2D, structure.font_texture);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGB,
      texture_width,
      texture_height,
      0,
      GL_RGB,
      GL_UNSIGNED_BYTE,
      0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Bind the font to a framebuffer to draw to

  unsigned int framebuffer;
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture(
      GL_FRAMEBUFFER,
      GL_COLOR_ATTACHMENT0,
      structure.font_texture,
      0);

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
      throw std::runtime_error(
          "Failed to load character: " + std::to_string(char(i)));
    }

    auto& character = structure.get(i);

    // Calculate where the character should be drawn on the texture

    if (char_x + character.advance > texture_width) {
      char_x = 0;
      char_y += structure.line_height;
    }

    float x_lower = char_x + character.offset.x;
    float x_upper = x_lower + ft_face->glyph->bitmap.width;
    float y_lower = char_y + structure.descender + character.offset.y;
    float y_upper = y_lower + ft_face->glyph->bitmap.rows;

    Vecf bottom_left(
        -1.f + 2 * x_lower / texture_width,
        -1.f + 2 * y_lower / texture_height);
    Vecf top_right(
        -1.f + 2 * x_upper / texture_width,
        -1.f + 2 * y_upper / texture_height);
    Vecf bottom_right(top_right.x, bottom_left.y);
    Vecf top_left(bottom_left.x, top_right.y);

    character.uv = Boxf(
        Vecf(x_lower / texture_width, y_lower / texture_height),
        Vecf(x_upper / texture_width, y_upper / texture_height));

    char_x += character.advance;

    // Load the vertices

    std::vector<Vertex> vertices = {
        Vertex{bottom_left, Vecf(0, 1)},
        Vertex{bottom_right, Vecf(1, 1)},
        Vertex{top_left, Vecf(0, 0)},
        Vertex{bottom_right, Vecf(1, 1)},
        Vertex{top_right, Vecf(1, 0)},
        Vertex{top_left, Vecf(0, 0)}};

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_STATIC_DRAW);
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
        ft_face->glyph->bitmap.buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &char_texture);
  }

  // Cleanup

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &framebuffer);
  glDeleteProgram(shader_program);
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  FT_Done_Face(ft_face);
  FT_Done_FreeType(ft_library);

  structure.font_texture_width = texture_width;
  structure.font_texture_height = texture_height;
  return structure;
}

const FontStructure& FontManager::font_structure(Font font, int font_size) {
  auto iter = fonts.find(std::make_pair(font, font_size));
  if (iter != fonts.end()) {
    return iter->second;
  }
  auto new_font = fonts.emplace(
      std::make_pair(font, font_size),
      load_font(font, font_size));
  return new_font.first->second;
};

Vecf FontManager::text_size(
    const std::string& text,
    const TextStyle& style,
    Length width) {
  const auto& fs = font_structure(style.font, style.font_size);

  auto fixed_width = std::get_if<LengthFixed>(&width);

  Vecf pos = Vecf::Zero();
  pos.y += fs.line_height;

  float line_break_max_x = 0;

  for (char c : text) {
    if (c == '\n') {
      line_break_max_x = std::max(pos.x, line_break_max_x);
      pos.x = 0;
      pos.y += fs.line_height;
      continue;
    }
    if (!fs.char_valid(c)) {
      continue;
    }
    const auto& character = fs.get(c);
    if (fixed_width && pos.x + character.advance > fixed_width->value) {
      pos.x = 0;
      pos.y += fs.line_height;
    }
    pos.x += character.advance;
  }

  if (fixed_width) {
    return Vecf(fixed_width->value, pos.y);
  } else {
    return Vecf(std::max(line_break_max_x, pos.x), pos.y);
  }
}

} // namespace datagui
