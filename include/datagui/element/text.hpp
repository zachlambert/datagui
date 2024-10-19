#pragma once

#include <string>
#include "datagui/style.hpp"
#include "datagui/internal/text.hpp"
#include "datagui/internal/tree.hpp"

namespace datagui {

struct Text {
    std::string text;
    float max_width;

    Text(
        const std::string& text,
        float max_width
    ):
        text(text),
        max_width(max_width)
    {}
};

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    const FontStructure& font,
    Node& node,
    const Text& element);

} // namespace datagui
