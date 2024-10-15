#pragma once

#include <string>
#include <stack>
#include <functional>
#include "datagui/geometry.hpp"
#include "datagui/internal/vector_map.hpp"


namespace datagui {

enum class Element {
    VerticalLayout,
    HorizontalLayout,
    Text,
    Button,
    Checkbox,
    TextInput
};

struct Node {
    // Definition
    std::string key;
    Element element;
    int element_index;

    // Connectivity
    int parent;
    int prev;
    int next;
    int first_child;
    int last_child;

    // Layout calculation
    int iteration;
    Vecf fixed_size;
    Vecf dynamic_size;
    Vecf origin;
    Vecf size;

    // State
    bool clicked;

    Node(const std::string& key, Element element, int parent, int iteration):
        key(key),
        element(element),
        element_index(-1),
        parent(parent),
        prev(-1),
        next(-1),
        first_child(-1),
        last_child(-1),
        iteration(iteration),
        fixed_size(Vecf::Zero()),
        dynamic_size(Vecf::Zero()),
        origin(Vecf::Zero()),
        size(Vecf::Zero()),
        clicked(false)
    {}

    void reset(int iteration) {
        this->iteration = iteration;
        fixed_size = Vecf::Zero();
        dynamic_size = Vecf::Zero();
        origin = Vecf::Zero();
        size = Vecf::Zero();
    }
};

class Tree {
public:
    using construct_element_t = std::function<int()>;
    using delete_element_t = std::function<void(Element, int)>;

    Tree(const delete_element_t& delete_element):
        delete_element(delete_element),
        root_node_(-1),
        max_depth_(0),
        iteration(0)
    {}

    // Define the tree
    int down(const std::string& key, Element element, const construct_element_t& construct_element);
    void up();
    void reset();

    const Node& operator[](std::size_t i) const {
        return nodes[i];
    }
    Node& operator[](std::size_t i) {
        return nodes[i];
    }

    std::size_t depth() const { return active_nodes.size(); }
    std::size_t max_depth() const { return max_depth_; }
    int root_node() const { return root_node_; }

private:
    void remove_node(int root_node);

    delete_element_t delete_element;
    VectorMap<Node> nodes;
    int root_node_;
    std::stack<int> active_nodes;
    std::size_t max_depth_;
    int iteration;
};

} // namespace datagui
