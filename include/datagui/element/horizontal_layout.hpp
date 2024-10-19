#pragma once

#include "datagui/internal/element.hpp"

namespace datagui {

class HorizontalLayout: public ElementInterface {
public:
    HorizontalLayout(const Vecf& input_size):
        input_size(input_size)
    {}

    virtual void calculate_size_components(
        const Style& style,
        const FontStructure& font,
        Node& node,
        const Tree& tree) const override;

    // Container elements only
    void calculate_child_dimensions(
        const Style& style,
        const Node& node,
        Tree& tree) const override;

    void render(
        const Style& style,
        const FontStructure& font,
        const Node& node,
        const NodeState& state,
        const TextSelection& selection,
        Renderers& renderers) const override;

private:
    Vecf input_size;
};

} // namespace datagui
