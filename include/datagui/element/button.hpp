#pragma once

#include <string>
#include "datagui/element/element.hpp"


namespace datagui {

class Button: public ElementInterface {
public:
    Button(
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
        Renderers& renderers) const override;

private:
    std::string text;
    float max_width;
};

} // namespace datagui
