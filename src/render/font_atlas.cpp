#include "datagui/render/font_atlas.hpp"
#include "datagui/render/lookup/ansi_colors.hpp"
#include "datagui/render/program/font_program.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>
#include <utility>

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
}

namespace dgui {

static constexpr int CHAR_BEGIN = ' ';
static constexpr int CHAR_END = '~' + 1;

static constexpr float GLYPH_PADDING_H = 2;
static constexpr float GLYPH_PADDING_V = 2;

FontAtlas::FontAtlas(
    FontProgram& program,
    const std::string& font_path,
    int font_size) {

  // Cache the GL state, so it can be restored on exit

  GLint original_fb;
  GLint original_viewport[4];
  GLint original_program;
  GLint original_vao;
  GLint original_unpack_alignment;
  GLboolean original_blend = glIsEnabled(GL_BLEND);
  GLint original_blend_src, original_blend_dst;

  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &original_fb);
  glGetIntegerv(GL_VIEWPORT, original_viewport);
  glGetIntegerv(GL_CURRENT_PROGRAM, &original_program);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &original_vao);
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &original_unpack_alignment);
  glGetIntegerv(GL_BLEND_SRC_RGB, &original_blend_src);
  glGetIntegerv(GL_BLEND_DST_RGB, &original_blend_dst);

  // Temporary state used for the FT_Library / framebuffer

  FT_Library library = nullptr;
  FT_Face face = nullptr;
  unsigned int framebuffer = 0;

  // Must be called on every exit path from the constructor. On failure, the
  // atlas texture is also freed, since the destructor isn't called for a
  // partially constructed object.
  auto cleanup = [&](bool success) {
    // The face is owned by the library, so it must be destroyed first;
    // FT_Done_FreeType would otherwise already have freed it.
    if (face) {
      FT_Done_Face(face);
    }
    if (library) {
      FT_Done_FreeType(library);
    }
    if (framebuffer) {
      glDeleteFramebuffers(1, &framebuffer);
    }
    if (!success && texture_) {
      glDeleteTextures(1, &texture_);
      texture_ = 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, original_fb);
    glViewport(
        original_viewport[0],
        original_viewport[1],
        original_viewport[2],
        original_viewport[3]);
    glUseProgram(original_program);
    glBindVertexArray(original_vao);
    glPixelStorei(GL_UNPACK_ALIGNMENT, original_unpack_alignment);
    if (original_blend) {
      glEnable(GL_BLEND);
    } else {
      glDisable(GL_BLEND);
    }
    glBlendFunc(original_blend_src, original_blend_dst);
  };

  // Return zero for success, >0 for failure
  if (FT_Init_FreeType(&library)) {
    cleanup(false);
    throw std::runtime_error("Failed to initialize freetype library");
  }
  if (FT_New_Face(library, font_path.c_str(), 0, &face)) {
    cleanup(false);
    throw std::runtime_error("Failed to load font '" + font_path + "'");
  }
  FT_Set_Pixel_Sizes(face, 0, font_size);

  glGenFramebuffers(1, &framebuffer);

  // Initialize geometric properties
  line_height_ = font_size * float(face->height) / face->units_per_EM;
  ascender_ = font_size * float(face->ascender) / face->units_per_EM;
  descender_ = -font_size * float(face->descender) / face->units_per_EM;

  // 1st pass iterate through characters
  // -> Read properties and find required texture height

  texture_width_ = 512;
  texture_height_ =
      line_height_ + GLYPH_PADDING_V; // Initial value, at least 1 line needed

  glyphs_.reserve(CHAR_END - CHAR_BEGIN);

  float texture_row_width = GLYPH_PADDING_H;
  for (int i = CHAR_BEGIN; i < CHAR_END; i++) {
    if (FT_Load_Char(face, char(i), 0) != 0) {
      cleanup(false);
      throw std::runtime_error(
          "Failed to load character: " + std::to_string(char(i)));
    }

    FontGlyph& glyph = glyphs_.emplace_back();
    glyph.size = Vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
    glyph.offset = Vec2(
        face->glyph->bitmap_left,
        float(face->glyph->bitmap_top) - face->glyph->bitmap.rows);
    glyph.advance = float(face->glyph->advance.x) / 64;

    if (texture_row_width + (glyph.size.x + GLYPH_PADDING_H) > texture_width_) {
      texture_height_ += line_height_ + GLYPH_PADDING_V;
      texture_row_width = GLYPH_PADDING_H;
    }
    texture_row_width += glyph.size.x + GLYPH_PADDING_H;
  }
  texture_height_ += GLYPH_PADDING_V;

  // Create the atlas texture

  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_R8,
      texture_width_,
      texture_height_,
      0,
      GL_RED,
      GL_UNSIGNED_BYTE,
      0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Bind the texture to the temporary framebuffer

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_, 0);

  // 2nd pass iterate through characters
  // -> Render to the atlas texture

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, texture_width_, texture_height_);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

  program.bind();

  float char_x = GLYPH_PADDING_H;
  float char_y = GLYPH_PADDING_V;
  for (int i = CHAR_BEGIN; i < CHAR_END; i++) {
    if (FT_Load_Char(face, char(i), FT_LOAD_RENDER) != 0) {
      cleanup(false);
      throw std::runtime_error(
          "Failed to load character: " + std::to_string(char(i)));
    }

    FontGlyph& glyph = glyphs_[i - CHAR_BEGIN];

    // Calculate where the character should be drawn on the texture

    if (char_x + (glyph.size.x + GLYPH_PADDING_H) > texture_width_) {
      char_x = GLYPH_PADDING_H;
      char_y += line_height_ + GLYPH_PADDING_V;
    }

    float x_lower = char_x;
    float x_upper = x_lower + face->glyph->bitmap.width;
    float y_lower = char_y + descender_ + glyph.offset.y;
    float y_upper = y_lower + face->glyph->bitmap.rows;

    Box2 box;
    box.lower.x = -1.f + 2 * x_lower / texture_width_;
    box.lower.y = -1.f + 2 * y_lower / texture_height_;
    box.upper.x = -1.f + 2 * x_upper / texture_width_;
    box.upper.y = -1.f + 2 * y_upper / texture_height_;

    glyph.uv = Box2(
        Vec2(x_lower / texture_width_, y_lower / texture_height_),
        Vec2(x_upper / texture_width_, y_upper / texture_height_));

    char_x += (glyph.size.x + GLYPH_PADDING_H);

    program.draw_bitmap(
        box,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        face->glyph->bitmap.buffer);
  }

  cleanup(true);
}

FontAtlas::~FontAtlas() {
  if (texture_) {
    glDeleteTextures(1, &texture_);
  }
}

FontAtlas::FontAtlas(FontAtlas&& other) noexcept :
    line_height_(other.line_height_),
    ascender_(other.ascender_),
    descender_(other.descender_),
    texture_(std::exchange(other.texture_, 0)),
    texture_width_(other.texture_width_),
    texture_height_(other.texture_height_),
    glyphs_(std::move(other.glyphs_)) {}

FontAtlas& FontAtlas::operator=(FontAtlas&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  if (texture_) {
    glDeleteTextures(1, &texture_);
  }
  line_height_ = other.line_height_;
  ascender_ = other.ascender_;
  descender_ = other.descender_;
  texture_ = std::exchange(other.texture_, 0);
  texture_width_ = other.texture_width_;
  texture_height_ = other.texture_height_;
  glyphs_ = std::move(other.glyphs_);
  return *this;
}

Vec2 FontAtlas::text_size(const std::string& text, Length width, bool editable)
    const {
  auto fixed_width = std::get_if<LengthFixed>(&width);

  Vec2 size;
  size.y += line_height_;

  float line_break_max_x = 0;

  for (size_t i = 0; i < text.size(); i++) {
    char c = text[i];
    if (c == '\n') {
      line_break_max_x = std::max(size.x, line_break_max_x);
      size.x = 0;
      size.y += line_height_;
      continue;
    }
    if (!editable) {
      if (size_t n = ansi_sequence_match(&text[i], text.size() - i)) {
        i += (n - 1);
        continue;
      }
    }
    if (int(c) < CHAR_BEGIN || int(c) >= CHAR_END) {
      // Invalid character
      continue;
    }
    const auto& glyph = glyphs_[int(c) - CHAR_BEGIN];
    if (fixed_width && size.x + glyph.advance > fixed_width->value) {
      size.x = 0;
      size.y += line_height_;
    }
    size.x += glyph.advance;
  }

  if (fixed_width) {
    return Vec2(fixed_width->value, size.y);
  } else {
    return Vec2(std::max(line_break_max_x, size.x), size.y);
  }
}

float FontAtlas::text_height() const {
  return line_height_;
}

float FontAtlas::advance(char c) const {
  if (int(c) < CHAR_BEGIN || int(c) >= CHAR_END) {
    return 0;
  }
  return glyphs_[int(c) - CHAR_BEGIN].advance;
}

size_t FontAtlas::add_glyphs(
    std::vector<Glyph2dInstance>& instances,
    const Vec2& origin,
    double angle,
    const Vec2& scale,
    bool y_flipped,
    const Color& default_color,
    const std::string& text,
    Length width,
    bool editable) const {

  Color color = default_color;
  auto fixed_width = std::get_if<LengthFixed>(&width);

  Vec2 offset;
  offset.y -= line_height_;

  size_t instance_count = 0;
  for (size_t i = 0; i < text.size(); i++) {
    char c = text[i];
    if (c == '\n') {
      offset.x = 0;
      offset.y -= line_height_;
      continue;
    }
    if (!editable) {
      if (size_t n = ansi_sequence_match(
              &text[i],
              text.size() - i,
              default_color,
              color)) {
        i += (n - 1);
        continue;
      }
    }
    if (int(c) < CHAR_BEGIN || int(c) >= CHAR_END) {
      // Invalid character
      continue;
    }
    const auto& glyph = glyphs_[int(c) - CHAR_BEGIN];

    if (fixed_width && offset.x + glyph.advance > fixed_width->value) {
      offset.x = 0;
      offset.y -= line_height_;
    }

    Vec2 position = offset + glyph.offset + Vec2(0, descender_);
    if (y_flipped) {
      position.y *= -1;
    }

    Rot2 rotation(angle);
    Mat3 M = Mat3::transform(
        origin + rotation * (scale * position),
        rotation,
        scale * glyph.size * Vec2(1, y_flipped ? -1.f : 1.f));

    instances.push_back(
        Glyph2dInstance{M, glyph.uv.lower, glyph.uv.size(), color});
    instance_count++;

    offset.x += glyph.advance;
  }
  return instance_count;
}

std::size_t FontAtlas::find_cursor(
    const std::string& text,
    Length text_width,
    const Vec2& point) const {

  auto fixed_width = std::get_if<LengthFixed>(&text_width);
  Vec2 pos;
  pos.y += line_height_;

  std::size_t column = 0;
  bool column_found = false;

  for (std::size_t i = 0; i < text.size(); i++) {
    char c = text[i];
    if (c == '\n') {
      pos.x = 0;
      pos.y += line_height_;
      continue;
    }
    if (size_t n = ansi_sequence_match(&text[i], text.size() - i)) {
      i += (n - 1);
      continue;
    }
    if (int(c) < CHAR_BEGIN || int(c) >= CHAR_END) {
      // Invalid character
      continue;
    }
    const auto& glyph = glyphs_[int(c) - CHAR_BEGIN];

    if (!column_found && pos.x + glyph.advance / 2 > point.x) {
      column_found = true;
      column = i;
      if (point.y < pos.y) {
        return column;
      }
    }

    if (fixed_width && pos.x + glyph.advance > fixed_width->value) {
      if (!column_found) {
        column = i + 1;
      }
      if (point.y < pos.y) {
        return column;
      }
      pos.x = 0;
      pos.y += line_height_;
      column_found = false;
    }
    pos.x += glyph.advance;
  }
  if (!column_found) {
    column = text.size();
  }
  return column;
}

Vec2 FontAtlas::cursor_offset(
    const std::string& text,
    Length text_width,
    std::size_t cursor) const {
  auto fixed_width = std::get_if<LengthFixed>(&text_width);
  Vec2 offset;

  for (std::size_t i = 0; i < cursor; i++) {
    char c = text[i];
    if (c == '\n') {
      offset.x = 0;
      offset.y += line_height_;
      continue;
    }
    if (size_t n = ansi_sequence_match(&text[i], text.size() - i)) {
      i += (n - 1);
      continue;
    }
    if (int(c) < CHAR_BEGIN || int(c) >= CHAR_END) {
      // Invalid character
      continue;
    }
    const auto& glyph = glyphs_[int(c) - CHAR_BEGIN];

    if (fixed_width && offset.x + glyph.advance > fixed_width->value) {
      offset.x = 0;
      offset.y += line_height_;
    }
    offset.x += glyph.advance;
  }
  return offset;
}

} // namespace dgui
