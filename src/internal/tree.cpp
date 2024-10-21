#include "datagui/internal/tree.hpp"
#include "datagui/exception.hpp"
#include <assert.h>


namespace datagui {

Tree::Tree(
        const get_elements_t& get_elements):
    get_elements(get_elements),
    root_node_(-1),
    iteration(0),
    node_held_(-1),
    node_focused_(-1)
{}

void Tree::begin() {
    iteration++;
}

int Tree::next(
    const std::string& key,
    Element element,
    const construct_element_t& construct_element)
{
    if (active_nodes.empty()) {
        if (!key.empty()) {
            throw WindowError("Root node must be unnamed");
        }
        if (root_node_ == -1) {
            root_node_ = nodes.emplace(key, element, -1, iteration);
            nodes[root_node_].element_index = construct_element();
        }  else if (nodes[root_node_].iteration == iteration) {
            throw WindowError(
                "Visited the root node twice during the same iteration");
        }
        return root_node_;
    }

    int prev = active_nodes.top();
    active_nodes.pop();
    int parent = active_nodes.empty() ? root_node_ : active_nodes.top();
    assert(parent != -1);

    int node = prev == -1 ? nodes[parent].first_child : nodes[prev].next;

    while (node != -1) {
        if (key == nodes[node].key) {
            // Either both are unnamed (empty key) or named with the same key
            break;
        }
        node = nodes[node].next;
    }

    if (node == -1) {
        node = create_node(key, element, parent, prev);
        if (construct_element) {
            nodes[node].element_index = construct_element();
        } else {
            nodes[node].hidden = true;
        }
    } else {
        nodes[node].reset(iteration);
        if (nodes[node].element_index == -1 && construct_element) {
            nodes[node].element = element;
            nodes[node].element_index = construct_element();
        } else if (!construct_element) {
            nodes[node].hidden = true;
        }
    }

    if (nodes[node].element_index == -1) {
        nodes[node].hidden = true;
    }
    active_nodes.push(node);

    return node;
}

void Tree::down() {
    active_nodes.push(-1);
}

void Tree::up(bool skipped) {
    if (active_nodes.empty()) {
        throw std::runtime_error("Called end too many times");
    }

    if (skipped) {
        active_nodes.pop();
        return;
    }

    // Remove nodes that weren't visited this iteration

    active_nodes.pop();
    int parent = active_nodes.empty() ? root_node_ : active_nodes.top();
    int iter = nodes[parent].first_child;
    while (iter != -1) {
        int next = nodes[iter].next;
        if (nodes[iter].iteration != iteration) {
            remove_node(iter);
        } else {
            nodes[iter].changed = false;
        }
        iter = next;
    }
}

int Tree::create_node(
    const std::string& key,
    Element element,
    int parent,
    int prev)
{
    int node = nodes.emplace(key, element, parent, iteration);

    nodes[node].prev = prev;
    int next = prev == -1 ? nodes[parent].first_child : nodes[prev].next;
    nodes[node].next = next;

    if (prev != -1) {
        nodes[prev].next = node;
    } else {
        nodes[parent].first_child = node;
    }
    if (next != -1) {
        nodes[next].prev = node;
    } else {
        nodes[parent].last_child = node;
    }
    return node;
}

void Tree::remove_node(int root_node) {
    std::stack<int> stack;
    stack.push(root_node);
    while (!stack.empty()) {
        int node_index = stack.top();
        const auto& node = nodes[node_index];
        if (node.first_child == -1) {
            stack.pop();
            if (node.prev != -1) {
                nodes[node.prev].next = node.next;
            } else if (node.parent != -1) {
                nodes[node.parent].first_child = node.next;
            }
            if (node.next != -1) {
                nodes[node.next].prev = node.prev;
            } else if (node.parent != -1) {
                nodes[node.parent].last_child = node.prev;
            }
            get_elements(node).pop(node.element_index);
            nodes.pop(node_index);

            if (node_held_ == node_index) {
                node_held_ = -1;
            }
            if (node_focused_ == node_index) {
                node_focused_ = -1;
            }

            continue;
        }
        int iter = node.first_child;
        while (iter != -1) {
            stack.push(iter);
            iter = nodes[iter].next;
        }
    }
}

void Tree::end(const Vecf& root_size) {
    if (!active_nodes.empty()) {
        throw WindowError("Didn't call layout_... and layout_end the same number of times");
    }
    if (root_node_ == -1) {
        return;
    }
    nodes[root_node_].changed = false;

    // Calculate size components
    {
        struct State {
            std::size_t index;
            bool first_visit;
            State(int index):
                index(index),
                first_visit(true)
            {}
        };
        std::stack<State> stack;
        stack.emplace(root_node_);

        while (!stack.empty()) {
            State& state = stack.top();
            Node& node = nodes[state.index];

            if (node.hidden) {
                stack.pop();
                continue;
            }

            // If the node has children, process these first
            if (node.first_child != -1 && state.first_visit) {
                state.first_visit = false;
                int child = node.first_child;
                while (child != -1) {
                    stack.emplace(child);
                    child = nodes[child].next;
                }
                continue;
            }
            stack.pop();

            get_elements(node).calculate_size_components(node, *this);
        }
    }

    {
        std::stack<int> stack;

        stack.push(root_node_);
        nodes[root_node_].size = root_size;

        while (!stack.empty()) {
            const auto& node = nodes[stack.top()];
            stack.pop();

            if (node.hidden || node.first_child == -1) {
                continue;
            }

            get_elements(node).calculate_child_dimensions(node, *this);

            int child = node.first_child;
            while (child != -1) {
                stack.push(child);
                child = nodes[child].next;
            }
        }
    }
}

void Tree::render(Renderers& renderers) {
    if (root_node_ == -1) {
        return;
    }
    std::stack<int> stack;
    stack.push(root_node_);

    while (!stack.empty()) {
        int node_index = stack.top();
        const auto& node = nodes[stack.top()];
        stack.pop();

        if (node.hidden) {
            continue;
        }
        get_elements(node).render(node, node_state(node_index), renderers);
        if (node.first_child == -1) {
            continue;
        }
        int child = node.first_child;
        while (child != -1) {
            stack.push(child);
            child = nodes[child].next;
        }
    }
}

void Tree::mouse_press(const Vecf& mouse_pos) {
    if (root_node_ == -1) {
        return;
    }

    int node_pressed = root_node_;

    while (true) {
        const auto& node = nodes[node_pressed];
        if (node.hidden) {
            break;
        }
        int child_index = node.first_child;
        while (child_index != -1) {
            const auto& child = nodes[child_index];
            if (Boxf(child.origin, child.origin+child.size).contains(mouse_pos)) {
                node_pressed = child_index;
                break;
            }
            child_index = child.next;
        }
        if (child_index == -1) {
            break;
        }
    }

    auto& node = nodes[node_pressed];
    if (get_elements(node).press(node, mouse_pos)) {
        node_changed(node);
    }

    node_held_ = node_pressed;
    if (node_focused_ != -1 && node_pressed != node_focused_) {
        auto& released = nodes[node_focused_];
        if (get_elements(released).focus_leave(released, true)) {
            node_changed(released);
        }
    }
    node_focused_ = node_pressed;
}

void Tree::mouse_release(const Vecf& mouse_pos) {
    if (node_held_ == -1) {
        return;
    }
    auto& node = nodes[node_held_];
    if (!node.hidden && Boxf(node.origin, node.origin+node.size).contains(mouse_pos)) {
        if (get_elements(node).release(node, mouse_pos)) {
            node_changed(node);
        }
    }
    node_held_ = -1;
}

void Tree::focus_next() {
    int next = node_focused_;

    if (next == -1) {
        next = root_node_;
    } else if (nodes[next].first_child != -1) {
        // 1. If not focused on a leaf node, change to the first encountered
        // leaf node (depth first)
        while (nodes[next].first_child != -1) {
            next = nodes[next].first_child;
        }
    } else if (nodes[next].next != -1) {
        // 2. If there is an immediate neighbour, change to this
        next = nodes[next].next;
    } else {
        // 3. No immediate neighbour, so two steps:
        // - Move up the tree until reaching a node with a next, or reaching
        //   the root node
        // - Move down the tree to the first leaf node in that subtree
        while (nodes[next].next == -1) {
            next = nodes[next].parent;
            if (next == -1) {
                break;
            }
        }
        if (next != -1) {
            next = nodes[next].next;
            while (nodes[next].first_child != -1) {
                next = nodes[next].first_child;
            }
        }
    }

    if (node_focused_ != -1) {
        auto& prev_focused = nodes[node_focused_];
        if (get_elements(prev_focused).focus_leave(prev_focused, true)) {
            node_changed(prev_focused);
        }
    }
    if (next != -1) {
        auto& new_focused = nodes[next];
        if (get_elements(new_focused).focus_enter(new_focused)) {
            node_changed(new_focused);
        }
    }

    node_focused_ = next;
}

void Tree::focus_leave(bool success) {
    if (node_focused_ == -1) {
        return;
    }
    auto& node = nodes[node_focused_];
    if (get_elements(node).focus_leave(node, success)) {
        node_changed(node);
    }
    node_focused_ = -1;
}

NodeState Tree::node_state(int node) const {
    NodeState state;
    state.held = (node == node_held_);
    state.focused = (node == node_focused_);
    return state;
}

void Tree::node_changed(Node& node) {
    node.changed = true;
    int iter = node.parent;
    while (iter != -1) {
        nodes[iter].changed = true;
        iter = nodes[iter].parent;
    }
}

} // namespace datagui
