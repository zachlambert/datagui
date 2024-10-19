#pragma once

#include <string>
#include "datagui/internal/element.hpp"


namespace datagui {

struct TextInput {
    float max_width;
    std::string initial_text;
    std::string text;
    bool changed;

    TextInput(float max_width, const std::string& default_text):
        max_width(max_width),
        initial_text(default_text),
        text(default_text),
        changed(false)
    {}
};

class TextInputSystem: public ElementSystem {
public:
    TextInputSystem(
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

    int create(float max_width, const std::string& default_text) {
        return elements.emplace(max_width, default_text);
    }

    void pop(int index) override {
        elements.pop(index);
    }

private:
    const Style& style;
    const FontStructure& font;
    TextSelection text_selection;
    VectorMap<TextInput> elements;
};

} // namespace datagui
