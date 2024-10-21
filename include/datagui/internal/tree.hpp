#pragma once

#include <string>
#include <stack>
#include <functional>
#include "datagui/geometry.hpp"
#include "datagui/internal/vector_map.hpp"
#include "datagui/internal/renderers.hpp"
#include "datagui/internal/element.hpp"


namespace datagui {

enum class Element {
    Button,
    Checkbox,
    LinearLayout,
    Selection,
    Text,
    TextInput
};

struct NodeState {
    bool held;
    bool focused;

    NodeState():
        held(false),
        focused(false)
    {}
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
    Vecf fixed_size;
    Vecf dynamic_size;
    bool floating;
    Vecf origin;
    Vecf size;

    // State
    bool changed;
    bool hidden;

    Node(const std::string& key, Element element, int parent):
        key(key),
        element(element),
        element_index(-1),
        parent(parent),
        prev(-1),
        next(-1),
        first_child(-1),
        last_child(-1),
        fixed_size(Vecf::Zero()),
        dynamic_size(Vecf::Zero()),
        floating(false),
        origin(Vecf::Zero()),
        size(Vecf::Zero()),
        changed(true),
        hidden(false)
    {}
};

class Tree {
public:
    using construct_element_t = std::function<int()>;
    using get_elements_t = std::function<ElementSystem&(const Node&)>;

    Tree(const get_elements_t& get_elements);

    // Define the tree
    void begin();
    int next(
        const std::string& key,
        Element element,
        const construct_element_t& construct_element);
    void down();
    void up();
    void end(const Vecf& root_size);

    void render(Renderers& renderers);

    const Node& operator[](std::size_t i) const {
        return nodes[i];
    }
    Node& operator[](std::size_t i) {
        return nodes[i];
    }

    int root_node() const { return root_node_; }

    void mouse_press(const Vecf& mouse_pos);
    void mouse_release(const Vecf& mouse_pos);
    void focus_next(bool reverse=false);
    void focus_leave(bool success);

    int node_held() const { return node_held_; }
    int node_focused() const { return node_focused_; }

    void node_changed(Node& node);

private:
    NodeState node_state(int node) const;
    int create_node(
        const std::string& key,
        Element element,
        int parent,
        int prev);
    void remove_node(int root_node);

    get_elements_t get_elements;

    VectorMap<Node> nodes;
    int root_node_;
    int parent;
    int current;

    int node_held_;
    int node_focused_;
};

} // namespace datagui
