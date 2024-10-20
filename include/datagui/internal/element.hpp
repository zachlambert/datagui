#pragma once

#include "datagui/geometry.hpp"
#include "datagui/internal/selection.hpp"


namespace datagui {

class Tree;
class Node;
class NodeState;
class Renderers;

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

    virtual void focus_enter(const Node& node) {};
    virtual void focus_leave(const Node& node, bool success) {};
    virtual void key_event(const Node& node, const KeyEvent& event) {};
};

} // namespace datagui
