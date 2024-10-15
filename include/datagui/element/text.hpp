#pragma once

#include <string>
#include "datagui/element.hpp"
#include "datagui/style.hpp"

namespace datagui {

class TextRenderer;

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
    const TextRenderer& text_renderer,
    Node& node,
    const Text& element);

} // namespace datagui
