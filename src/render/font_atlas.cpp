#include "datagui/render/font_atlas.hpp"
#include "datagui/render/font_program.hpp"
#include "datagui/visual/shader_utils.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
}

namespace dgui {

static constexpr int CHAR_BEGIN = ' ';
static constexpr int CHAR_END = '~' + 1;

FontAtlas::FontAtlas(
    FontProgram& program,
    const std::string& font_path,
    int font_size) {
  // Keep track of these values to restore afterwards

  int original_fb;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &original_fb);

  int original_fb_w, original_fb_h;
  int original_viewport[4];
  glGetIntegerv(GL_VIEWPORT, original_viewport);

  unsigned char original_blend;
  glGetBooleanv(GL_BLEND, &original_blend);

  // Initialise ft_library

  FT_Library ft_library;
  if (FT_Init_FreeType(&ft_library) != 0) {
    throw std::runtime_error("Failed to initialize freetype library");
  }

  FT_Face ft_face;
  if (FT_New_Face(ft_library, font_path.c_str(), 0, &ft_face) != 0) {
    throw std::runtime_error("Failed to load font '" + font_path + "'");
  }

  FT_Set_Pixel_Sizes(ft_face, 0, font_size);

  // Initialize geometric properties
  line_height_ = float(ft_face->height) / 128;
  ascender_ = float(ft_face->ascender) / 128;
  descender_ = -float(ft_face->descender) / 128;

  // For some reason, the ascender, descender and line_height values aren't
  // scaled to the requested font size, even though the glyphs are
  float scale = float(font_size) / line_height_;
  ascender_ *= scale;
  descender_ *= scale;
  line_height_ = font_size;

  // 1st pass iterate through characters
  // -> Read properties and find required texture height

  texture_width_ = 512;
  texture_height_ = line_height_; // Initial value, at least 1 line needed

  glyphs_.reserve(CHAR_END - CHAR_BEGIN);

  float texture_row_width = 0;
  for (int i = CHAR_BEGIN; i < CHAR_END; i++) {
    if (FT_Load_Char(ft_face, char(i), 0) != 0) {
      throw std::runtime_error(
          "Failed to load character: " + std::to_string(char(i)));
    }

    FontGlyph& glyph = glyphs_.emplace_back();
    glyph.size =
        Vec2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows);
    glyph.offset = Vec2(
        ft_face->glyph->bitmap_left,
        float(ft_face->glyph->bitmap_top) - ft_face->glyph->bitmap.rows);
    glyph.advance = float(ft_face->glyph->advance.x) / 64;

    if (texture_row_width + glyph.advance > texture_width_) {
      texture_height_ += line_height_;
      texture_row_width = 0;
    }
    texture_row_width += glyph.advance;
  }

  // Create the atlas texture

  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGB,
      texture_width_,
      texture_height_,
      0,
      GL_RGB,
      GL_UNSIGNED_BYTE,
      0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Bind the texture to a framebuffer to draw to

  unsigned int framebuffer;
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_, 0);

  // 2nd pass iterate through characters
  // -> Render to the atlas texture

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, texture_width_, texture_height_);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

  program.bind();

  float char_x = 0;
  float char_y = 0;
  for (int i = CHAR_BEGIN; i < CHAR_END; i++) {
    if (FT_Load_Char(ft_face, char(i), FT_LOAD_RENDER) != 0) {
      throw std::runtime_error(
          "Failed to load character: " + std::to_string(char(i)));
    }

    FontGlyph& glyph = glyphs_[i - CHAR_BEGIN];

    // Calculate where the character should be drawn on the texture

    if (char_x + glyph.advance > texture_width_) {
      char_x = 0;
      char_y += line_height_;
    }

    float x_lower = char_x + glyph.offset.x;
    float x_upper = x_lower + ft_face->glyph->bitmap.width;
    float y_lower = char_y + descender_ + glyph.offset.y;
    float y_upper = y_lower + ft_face->glyph->bitmap.rows;

    Box2 box;
    box.lower.x = -1.f + 2 * x_lower / texture_width_;
    box.lower.y = -1.f + 2 * y_lower / texture_height_;
    box.upper.x = -1.f + 2 * x_upper / texture_width_;
    box.upper.y = -1.f + 2 * y_upper / texture_height_;

    glyph.uv = Box2(
        Vec2(x_lower / texture_width_, y_lower / texture_height_),
        Vec2(x_upper / texture_width_, y_upper / texture_height_));

    char_x += glyph.advance;

    program.draw_bitmap(
        box,
        ft_face->glyph->bitmap.width,
        ft_face->glyph->bitmap.rows,
        ft_face->glyph->bitmap.buffer);
  }

  // Cleanup

  glDeleteFramebuffers(1, &framebuffer);
  FT_Done_Face(ft_face);
  FT_Done_FreeType(ft_library);
}

Vec2 FontAtlas::text_size(const std::string& text, Length width) {
  auto fixed_width = std::get_if<LengthFixed>(&width);

  Vec2 pos;
  pos.y += line_height_;

  float line_break_max_x = 0;

  for (char c : text) {
    if (c == '\n') {
      line_break_max_x = std::max(pos.x, line_break_max_x);
      pos.x = 0;
      pos.y += line_height_;
      continue;
    }
    if (int(c) < CHAR_BEGIN || int(c) >= CHAR_END) {
      // Invalid character
      continue;
    }
    const auto& glyph = glyphs_[int(c) - CHAR_BEGIN];
    if (fixed_width && pos.x + glyph.advance > fixed_width->value) {
      pos.x = 0;
      pos.y += line_height_;
    }
    pos.x += glyph.advance;
  }

  if (fixed_width) {
    return Vec2(fixed_width->value, pos.y);
  } else {
    return Vec2(std::max(line_break_max_x, pos.x), pos.y);
  }
}

float FontAtlas::text_height() {
  return line_height_;
}

size_t FontAtlas::add_glyphs(
    std::vector<Glyph2dVertex>& vertices,
    const Vec2& origin,
    double angle,
    const Vec2& scale,
    const std::string& text,
    Length width) {

  auto fixed_width = std::get_if<LengthFixed>(&width);

  Vec2 offset;
  offset.y -= line_height_;

  size_t vertex_count = 0;
  for (char c : text) {
    if (c == '\n') {
      offset.x = 0;
      offset.y -= line_height_;
      continue;
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
    Box2 box(position, position + glyph.size);

    Rot2 rot(angle);
    Vec2 lower_left = origin + rot * scale * box.lower_left();
    Vec2 lower_right = origin + rot * scale * box.lower_right();
    Vec2 upper_left = origin + rot * scale * box.upper_left();
    Vec2 upper_right = origin + rot * scale * box.upper_right();

    vertices.emplace_back(lower_left, glyph.uv.lower_left());
    vertices.emplace_back(lower_right, glyph.uv.lower_right());
    vertices.emplace_back(upper_left, glyph.uv.upper_left());
    vertices.emplace_back(lower_right, glyph.uv.lower_right());
    vertices.emplace_back(upper_right, glyph.uv.upper_right());
    vertices.emplace_back(upper_left, glyph.uv.upper_left());

    vertex_count += 6;

    offset.x += glyph.advance;
  }
  return vertex_count;
}

} // namespace dgui
