#pragma once

#include <string>
#include "datagui/style.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/internal/text.hpp"
#include "datagui/internal/selection.hpp"
#include "datagui/internal/renderers.hpp"


namespace datagui {

class ElementInterface {
public:
    virtual void calculate_size_components(
        const Style& style,
        const FontStructure& font,
        Node& node,
        const Tree& tree) const = 0;

    // Container elements only
    virtual void calculate_child_dimensions(
        const Style& style,
        const Node& node,
        Tree& tree) const {}

    virtual void render(
        const Style& style,
        const FontStructure& font,
        const Node& node,
        const NodeState& state,
        const TextSelection& selection,
        Renderers& renderers) const = 0;
};

} // namespace datagui
