#pragma once

#include <string>
#include "datagui/element.hpp"
#include "datagui/style.hpp"
#include "datagui/internal/text.hpp"

namespace datagui {

struct Button {
    std::string text;
    float max_width;

    Button(
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
    const Button& element);

} // namespace datagui
