#include "datagui/render/font_registry.hpp"

namespace dgui {

void FontRegistry::init() {
  program.init();
}

const FontAtlas& FontRegistry::get_font(Font font, int font_size) {
  auto iter = fonts.find(FontKey{font, font_size});
  if (iter == fonts.end()) {
    std::string font_path = lookup_font(font);
    iter = fonts
               .emplace(
                   FontKey{font, font_size},
                   FontAtlas(program, font_path, font_size))
               .first;
  }
  return iter->second;
}

} // namespace dgui
