#pragma once

#include "datagui/geometry.hpp"
#include "datagui/layout.hpp"
#include "datagui/render/state/glyph_2d_instance.hpp"
#include <string>
#include <vector>

namespace dgui {

struct FontGlyph {
  Box2 uv;
  Vec2 size;
  Vec2 offset;
  float advance;
};

class FontProgram;

class FontAtlas {
public:
  FontAtlas(FontProgram& program, const std::string& font_path, int font_size);
  ~FontAtlas();

  FontAtlas(FontAtlas&& other) noexcept;
  FontAtlas& operator=(FontAtlas&& other) noexcept;
  FontAtlas(const FontAtlas& other) = delete;
  FontAtlas& operator=(const FontAtlas& other) = delete;

  Vec2 text_size(const std::string& text, Length width = LengthWrap()) const;
  float text_height() const;
  size_t add_glyphs(
      std::vector<Glyph2dInstance>& instances,
      const Vec2& origin,
      double angle,
      const Vec2& scale,
      bool y_flipped,
      const Color& default_color,
      const std::string& text,
      Length width = LengthWrap()) const;

  unsigned int texture() const {
    return texture_;
  }

private:
  float line_height_ = 0;
  float ascender_ = 0;
  float descender_ = 0;
  unsigned int texture_ = 0;
  std::size_t texture_width_ = 0;
  std::size_t texture_height_ = 0;
  std::vector<FontGlyph> glyphs_;
};

} // namespace dgui
