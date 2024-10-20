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

    virtual void press(
        const Node& node,
        const Vecf& mouse_pos) {}

    virtual void release(
        const Node& node,
        const Vecf& mouse_pos) {}

    virtual void held(
        const Node& node,
        const Vecf& mouse_pos) {}
};

} // namespace datagui
