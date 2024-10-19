#pragma once

#include <string>
#include "datagui/internal/element.hpp"
#include "datagui/internal/vector_map.hpp"


namespace datagui {

class ElementSystem {
public:
    virtual void calculate_size_components(
        Node& node,
        const Tree& tree) const = 0;

    virtual void render(
        const Node& node,
        const NodeState& state) const = 0;
};

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
        const FontStructure& font,
        GeometryRenderer& geometry_renderer,
        TextRenderer& text_renderer
    ):
        style(style),
        font(font),
        geometry_renderer(geometry_renderer),
        text_renderer(text_renderer)
    {}

    int create(const std::string& text, float max_width = 0) {
        return elements.emplace(text, max_width);
    }

    void pop(int index) {
        elements.pop(index);
    }

    void calculate_size_components(
        Node& node,
        const Tree& tree) const override;

    void render(
        const Node& node,
        const NodeState& state) const override;

private:
    const Style& style;
    const FontStructure& font;
    GeometryRenderer& geometry_renderer;
    TextRenderer& text_renderer;
    VectorMap<Button> elements;
};

} // namespace datagui
