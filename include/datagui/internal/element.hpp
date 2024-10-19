#pragma once

#include <string>
#include "datagui/style.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/internal/text.hpp"
#include "datagui/internal/selection.hpp"
#include "datagui/internal/renderers.hpp"


namespace datagui {

class ElementSystem {
public:
    virtual void calculate_size_components(
        Node& node,
        const Tree& tree) const = 0;

    virtual void calculate_child_dimensions(
        const Node& node,
        Tree& tree) const {}

    virtual void render(
        const Node& node,
        const NodeState& state,
        Renderers& renderers) const = 0;

    virtual void pop(int index) = 0;
};

#if 0

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

    // TODO
#if 0
    virtual void press(
        const Style& style,
        const FontStructure& font,
        const Node& node,
        const Vecf& mouse_pos) {}

    virtual void release(
        const Style& style,
        const FontStructure& font,
        const Node& node,
        const Vecf& mouse_pos) {}

    virtual void held(
        const Style& style,
        const FontStructure& font,
        const Node& node,
        const Vecf& mouse_pos) {}
#endif
};

#endif

} // namespace datagui
