#include "datagui/internal/tree.hpp"
#include "datagui/exception.hpp"


namespace datagui {

Tree::Tree(
        const get_elements_t& get_elements):
    get_elements(get_elements),
    root_node_(-1),
    max_depth_(0),
    iteration(0),
    node_held_(-1),
    node_focused_(-1)
{}

void Tree::begin() {
    iteration++;
    max_depth_ = 0;
}

int Tree::down(
    const std::string& key,
    Element type,
    const construct_element_t& construct_element)
{
    int parent = active_nodes.empty() ? -1 : active_nodes.top();

    if (parent != -1) {
        int iter = nodes[parent].first_child;
        while (iter != -1) {
            if (nodes[iter].key == key) {
                if (nodes[iter].iteration == iteration) {
                    throw WindowError("Visited the same node twice during the same iteration");
                }
                nodes[iter].reset(iteration);
                active_nodes.push(iter);
                max_depth_ = std::max<int>(max_depth_, active_nodes.size());
                return iter;
            }
            iter = nodes[iter].next;
        }
    } else if (root_node_ != -1) {
        active_nodes.push(root_node_);
        max_depth_ = std::max<int>(max_depth_, active_nodes.size());
        return root_node_;
    }

    int depth = (parent == -1) ? 0 : nodes[parent].depth + 1;
    int node = nodes.emplace(key, type, depth, parent, iteration);

    if (nodes.size() == 1) {
        root_node_ = node;
    }

    if (parent != -1) {
        nodes[node].prev = nodes[parent].last_child;
        if (nodes[parent].first_child == -1) {
            nodes[parent].first_child = node;
        } else {
            nodes[nodes[parent].last_child].next = node;
        }
        nodes[parent].last_child = node;
    }

    nodes[node].element_index = construct_element();

    active_nodes.push(node);
    max_depth_ = std::max<int>(max_depth_, active_nodes.size());

    return node;
}

void Tree::up() {
    if (active_nodes.empty()) {
        throw std::runtime_error("Called end too many times");
    }

    // Remove nodes that weren't visited this iteration

    int iter = nodes[active_nodes.top()].first_child;
    while (iter != -1) {
        int next = nodes[iter].next;
        if (nodes[iter].iteration != iteration) {
            remove_node(iter);
        }
        iter = next;
    }

    active_nodes.pop();
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
    if (depth() != 0) {
        throw WindowError("Didn't call layout_... and layout_end the same number of times");
    }
    if (root_node_ == -1) {
        return;
    }

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

            if (node.first_child == -1) {
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

        get_elements(node).render(node, node_state(node_index), renderers);
        if (node_focused_ == node_index) {
            renderers.geometry.queue_box(
                Boxf(node.origin, node.origin+node.size),
                Color(1, 0, 0, 0.2),
                0,
                Color::Black(),
                0);
        }

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

    const auto& node = nodes[node_pressed];
    get_elements(node).press(node, mouse_pos);

    node_held_ = node_pressed;
    if (node_focused_ != -1 && node_pressed != node_focused_) {
        const auto& released = nodes[node_focused_];
        get_elements(released).focus_leave(released, true);
    }
    node_focused_ = node_pressed;
}

void Tree::mouse_release(const Vecf& mouse_pos) {
    if (node_held_ == -1) {
        return;
    }
    const auto& node = nodes[node_held_];
    if (Boxf(node.origin, node.origin+node.size).contains(mouse_pos)) {
        get_elements(node).release(node, mouse_pos);
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
        const auto& prev_focused = nodes[node_focused_];
        get_elements(prev_focused).focus_leave(prev_focused, true);
    }
    if (next != -1) {
        const auto& new_focused = nodes[next];
        get_elements(new_focused).focus_enter(new_focused);
    }

    node_focused_ = next;
}

void Tree::focus_leave(bool success) {
    if (node_focused_ == -1) {
        return;
    }
    const auto& node = nodes[node_focused_];
    get_elements(node).focus_leave(node, success);
    node_focused_ = -1;
}

NodeState Tree::node_state(int node) const {
    NodeState state;
    state.held = (node == node_held_);
    state.focused = (node == node_focused_);
    return state;
}

} // namespace datagui
