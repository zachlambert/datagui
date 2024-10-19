#pragma once

#include <string>
#include "datagui/internal/element.hpp"
#include "datagui/internal/vector_map.hpp"


namespace datagui {

struct Button {
    std::string text;
    float max_width;

    Button(const std::string& text, float max_width):
        text(text), max_width(max_width)
    {}
};

class ButtonSystem: public ElementSystem {
public:
    ButtonSystem(
        const Style& style,
        const FontStructure& font
    ):
        style(style),
        font(font)
    {}

    void calculate_size_components(
        Node& node,
        const Tree& tree) const override;

    void render(
        const Node& node,
        const NodeState& state,
        Renderers& renderers) const override;

    int create(const std::string& text, float max_width) {
        return elements.emplace(text, max_width);
    }

    void pop(int index) override {
        elements.pop(index);
    }

private:
    const Style& style;
    const FontStructure& font;
    VectorMap<Button> elements;
};

} // namespace datagui
