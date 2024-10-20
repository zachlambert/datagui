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
    Text,
    TextInput
};

struct NodeState {
    bool pressed;
    bool released;
    bool held;
    bool focused;
    bool focus_released;

    NodeState():
        pressed(false),
        released(false),
        held(false),
        focused(false),
        focus_released(false)
    {}
};

struct Node {
    // Definition
    std::string key;
    Element element;
    int element_index;

    // Connectivity
    int depth;
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

    Node(const std::string& key, Element element, int depth, int parent, int iteration):
        key(key),
        element(element),
        element_index(-1),
        depth(depth),
        parent(parent),
        prev(-1),
        next(-1),
        first_child(-1),
        last_child(-1),
        iteration(iteration),
        fixed_size(Vecf::Zero()),
        dynamic_size(Vecf::Zero()),
        origin(Vecf::Zero()),
        size(Vecf::Zero())
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
    using get_elements_t = std::function<ElementSystem&(const Node&)>;
    using tick_focus_t = std::function<void(const Node&)>;

    Tree(const get_elements_t& get_elements, const tick_focus_t& tick_focus);

    // Define the tree
    void begin();
    int down(
        const std::string& key,
        Element element,
        const construct_element_t& construct_element);
    void up();
    void end(const Vecf& root_size);

    void render(Renderers& renderers);

    const Node& operator[](std::size_t i) const {
        return nodes[i];
    }
    Node& operator[](std::size_t i) {
        return nodes[i];
    }

    std::size_t depth() const { return active_nodes.size(); }
    std::size_t max_depth() const { return max_depth_; }
    int root_node() const { return root_node_; }

    void mouse_reset();
    void mouse_press(const Vecf& pos);
    void mouse_release(const Vecf& pos);
    void tick(const Vecf& mouse_pos);

    void focus_next();
    void focus_leave(bool success);

#if 0
    int node_pressed() const { return node_pressed_; }
    int node_released() const { return node_released_; }
    int node_held() const { return node_held_; }
    int node_focused() const { return node_focused_; }
    int node_focus_released() const { return node_focus_released_; }
#endif

private:
    NodeState node_state(int node) const;
    void remove_node(int root_node);

    get_elements_t get_elements;
    tick_focus_t tick_focus;

    VectorMap<Node> nodes;
    int root_node_;
    std::stack<int> active_nodes;
    std::size_t max_depth_;
    int iteration;

    int node_pressed_;
    int node_released_;
    int node_held_;
    int node_focused_;
    int node_focus_released_;
};

} // namespace datagui
