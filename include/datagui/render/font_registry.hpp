#pragma once

#include "datagui/font.hpp"
#include "datagui/render/font_atlas.hpp"
#include "datagui/render/program/font_program.hpp"
#include <unordered_map>

namespace dgui {

class FontRegistry {
public:
  void init();
  const FontAtlas& get_font(Font font, int font_size);

private:
  struct FontKey {
    Font font;
    int font_size;
    bool operator==(const FontKey& other) const {
      return font == other.font && font_size == other.font_size;
    }
  };
  struct FontHash {
    std::size_t operator()(const FontKey& key) const {
      return std::hash<int>{}(int(key.font) << 1) ^
             std::hash<int>{}(key.font_size);
    }
  };
  std::unordered_map<FontKey, FontAtlas, FontHash> fonts;

  FontProgram program;
};

} // namespace dgui
