#pragma once

#include "datagui/element.hpp"
#include "datagui/style.hpp"
#include <string>

namespace datagui {

class TextRenderer;

struct TextInput {
    float max_width;
    std::string initial_text;
    std::string text;
    bool changed;

    TextInput(
        const std::string& default_text,
        float max_width
    ):
        max_width(max_width),
        initial_text(default_text),
        text(default_text),
        changed(false)
    {}
};

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    const TextRenderer& text_renderer,
    Node& node,
    const TextInput& element);

} // namespace datagui
