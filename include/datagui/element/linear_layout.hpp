#pragma once

#include "datagui/internal/element.hpp"

namespace datagui {

enum class LayoutDirection {
    Horizontal,
    Vertical
};

struct LinearLayout {
    float length;
    float width;
    LayoutDirection direction;

    LinearLayout(float length, float width, LayoutDirection direction):
        length(length), width(width), direction(direction)
    {}
};

class LinearLayoutSystem: public ElementSystem {
public:
    LinearLayoutSystem(
        const Style& style
    ):
        style(style)
    {}

    int create(float length, float width, LayoutDirection direction) {
        return elements.emplace(length, width, direction);
    }

    void pop(int index) override {
        elements.pop(index);
    }

    void calculate_size_components(
        Node& node,
        const Tree& tree) const override;

    void calculate_child_dimensions(
        const Node& node,
        Tree& tree) const override;

    void render(
        const Node& node,
        const NodeState& state,
        Renderers& renderers) const override;

private:
    const Style& style;
    VectorMap<LinearLayout> elements;
};

} // namespace datagui
