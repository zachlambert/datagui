#include "datagui/element.hpp"
#include "datagui/exception.hpp"

namespace datagui {

Tree::Tree(const delete_element_t& delete_element):
    delete_element(delete_element),
    root_node_(-1),
    max_depth_(0),
    iteration(0),
    node_pressed_(-1),
    node_released_(-1),
    node_held_(-1),
    node_focused_(-1)
{}

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
            delete_element(node.element, node.element_index);
            nodes.pop(node_index);

            if (node_pressed_ == node_index) {
                node_pressed_ = -1;
            }
            if (node_released_ == node_index) {
                node_released_ = -1;
            }
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

void Tree::reset() {
    iteration++;
    max_depth_ = 0;
}

void Tree::mouse_reset() {
    node_pressed_ = -1;
    node_released_ = -1;
}

void Tree::mouse_press(const Vecf& pos) {
    node_pressed_ = root_node_;

    while (true) {
        const auto& node = nodes[node_pressed_];
        int child_index = node.first_child;
        while (child_index != -1) {
            const auto& child = nodes[child_index];
            if (Boxf(child.origin, child.origin+child.size).contains(pos)) {
                node_pressed_ = child_index;
                break;
            }
            child_index = child.next;
        }
        if (child_index == -1) {
            break;
        }
    }

    node_held_ = node_pressed_;
    node_focused_ = node_pressed_;
}

void Tree::mouse_release(const Vecf& pos) {
    if (node_held_ == -1) {
        return;
    }
    const auto& node = nodes[node_held_];
    if (Boxf(node.origin, node.origin+node.size).contains(pos)) {
        node_released_ = node_held_;
    }
    node_held_ = -1;
}

bool Tree::focus_next() {
    if (node_focused_ == -1) {
        return false;
    }
    const auto& node = nodes[node_focused_];
    if (node.next != -1) {
        node_focused_ = node.next;
        return true;
    } else if (node.parent != -1) {
        node_focused_ = nodes[node.parent].first_child;
        return true;
    } else {
        return false;
    }
}

void Tree::focus_escape() {
    node_focused_ = -1;
}

} // namespace datagui
