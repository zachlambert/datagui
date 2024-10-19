#pragma once

#include "datagui/internal/element.hpp"


namespace datagui {

struct Checkbox {
    bool checked;

    Checkbox(bool default_checked):
        checked(default_checked)
    {}
};

class CheckboxSystem: public ElementSystem {
public:
    CheckboxSystem(
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

    int create(bool default_checked) {
        return elements.emplace(default_checked);
    }

    void pop(int index) override {
        elements.pop(index);
    }

private:
    const Style& style;
    const FontStructure& font;
    VectorMap<Checkbox> elements;
};

} // namespace datagui
