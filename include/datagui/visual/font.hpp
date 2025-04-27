#pragma once

#include "datagui/geometry.hpp"
#include "datagui/style.hpp"
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

template <>
struct std::hash<std::pair<datagui::Font, int>> {
  using Key = std::pair<datagui::Font, int>;
  std::size_t operator()(const Key& k) const {
    return std::hash<int>()(int(k.first)) ^ (std::hash<int>()(k.second) << 1);
  }
};

namespace datagui {

struct FontStructure {
  struct Character {
    Boxf uv;
    Vecf size;
    Vecf offset;
    float advance;
  };
  float line_height;
  float ascender;
  float descender;
  unsigned int font_texture;
  std::size_t font_texture_width;
  std::size_t font_texture_height;

  FontStructure() :
      line_height(0),
      ascender(0),
      descender(0),
      font_texture(0),
      font_texture_width(0),
      font_texture_height(0),
      char_first_(0),
      char_end_(0) {}

  void resize(int char_first, int char_last) {
    char_first_ = char_first;
    char_end_ = char_last + 1;
    characters.resize(char_end_ - char_first_);
  }

  Character& get(int character) {
    if (character < char_first_ || character >= char_end_) {
      throw std::range_error("Invalid character");
    }
    return characters[character - char_first_];
  }

  const Character& get(int character) const {
    if (character < char_first_ || character >= char_end_) {
      throw std::range_error("Invalid character");
    }
    return characters[character - char_first_];
  }

  int char_first() const {
    return char_first_;
  }
  int char_end() const {
    return char_end_;
  }
  bool char_valid(char c) const {
    return int(c) >= char_first_ && int(c) < char_end_;
  }

private:
  int char_first_;
  int char_end_;
  std::vector<Character> characters;
};

class FontManager {
public:
  const FontStructure& font_structure(Font font, int font_size);
  Vecf text_size(const std::string& text, const TextStyle& style, Length width);

private:
  std::unordered_map<std::pair<Font, int>, FontStructure> fonts;
};

} // namespace datagui
