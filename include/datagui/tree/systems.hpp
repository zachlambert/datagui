#pragma once

#include "datagui/element/text.hpp"

namespace datagui {

struct Systems {
  Systems(FontManager& font_manager) : text(font_manager) {}

  TextSystem text;

  ElementSystem& operator[](ElementType type) {
    switch (type) {
    case ElementType::Text:
      return text;
    default:
      // TODO: Remove
      return text;
    }
  }

  const ElementSystem& operator[](ElementType type) const {
    return const_cast<Systems&>(*this)[type];
  }
};

} // namespace datagui
