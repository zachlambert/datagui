#pragma once

#include <string>
#include "datagui/internal/element.hpp"

namespace datagui {

struct Text {
    std::string text;
    float max_width;

    Text(const std::string& text, float max_width):
        text(text), max_width(max_width)
    {}
};

class TextSystem: public ElementSystem {
public:
    TextSystem(
        const Style& style,
        const FontStructure& font
    ):
        style(style),
        font(font)
    {}

    int create(const std::string& text, float max_width) {
        return elements.emplace(text, max_width);
    }

    void pop(int index) override {
        elements.pop(index);
    }

    void calculate_size_components(
        Node& node,
        const Tree& tree) const override;

    void render(
        const Node& node,
        const NodeState& state,
        Renderers& renderers) const override;

    void press(
        const Node& node,
        const Vecf& mouse_pos) override;

    void held(
        const Node& node,
        const Vecf& mouse_pos) override;

private:
    const Style& style;
    const FontStructure& font;
    TextSelection text_selection;
    VectorMap<Text> elements;
};

} // namespace datagui
