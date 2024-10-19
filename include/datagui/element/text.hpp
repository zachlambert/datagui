#pragma once

#include <string>
#include "datagui/internal/element.hpp"

namespace datagui {

class Text: public ElementInterface {
public:
    Text(
        const std::string& text,
        float max_width
    ):
        text(text),
        max_width(max_width)
    {}

    void calculate_size_components(
        const Style& style,
        const FontStructure& font,
        Node& node,
        const Tree& tree) const override;

    void render(
        const Style& style,
        const FontStructure& font,
        const Node& node,
        const NodeState& state,
        const TextSelection& selection,
        Renderers& renderers) const override;

    // TODO: Move text handling code to class
public:
    std::string text;
    float max_width;

};

} // namespace datagui
