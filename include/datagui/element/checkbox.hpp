#pragma once

#include "datagui/internal/element.hpp"


namespace datagui {

class Checkbox: public ElementInterface {
public:
    Checkbox(bool default_checked):
        checked_(default_checked)
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

    const bool& checked() const {
        return checked_;
    }

    void toggle() {
        checked_ = !checked_;
    }

private:
    bool checked_;
};

} // namespace datagui
