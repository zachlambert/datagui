#pragma once

#include <array>
#include <memory>
#include <stack>
#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <variant>

#include "datagui/style.hpp"
#include "datagui/geometry.hpp"
#include "datagui/internal/vector_map.hpp"
#include "datagui/internal/geometry_renderer.hpp"
#include "datagui/internal/text_renderer.hpp"

#include "datagui/element/button.hpp"
#include "datagui/element/checkbox.hpp"
#include "datagui/element/horizontal_layout.hpp"
#include "datagui/element/text.hpp"
#include "datagui/element/text_input.hpp"
#include "datagui/element/vertical_layout.hpp"


namespace datagui {

namespace element {

} // namespace element

class Window {
    enum class Element {
        VerticalLayout,
        HorizontalLayout,
        Text,
        Button,
        Checkbox,
        TextInput
    };

public:
    struct Config {
        std::string title;
        int width;
        int height;
        bool vsync;
        bool resizable;
        Config():
            title("datagui"),
            width(900),
            height(600),
            vsync(true),
            resizable(true)
        {}
    };

    Window(const Config& config = Config(), const Style& style = Style()):
        config(config),
        style(style),
        window(nullptr),
        root_node(-1),
        max_depth(0),
        iteration(0),
        node_pressed(-1),
        node_focused(-1)
    {}
    ~Window() {
        if (window) {
            close();
        }
    }

    bool running() const {
        return window && !glfwWindowShouldClose(window);
    }

    void open();
    void render_begin();
    void render_end();
    void close();

    void vertical_layout(
        const std::string& key,
        float width=0,
        float height=0);

    void horizontal_layout(
        const std::string& key,
        float width=0,
        float height=0);

    void layout_end();

    void text(
        const std::string& key,
        const std::string& text,
        float max_width = 0);

    bool button(
        const std::string& key,
        const std::string& text,
        float max_width = 0);

    bool checkbox(
        const std::string& key);

    bool text_input(
        const std::string& key,
        const std::string& default_text = "",
        float max_width = -1);

private:
    // Returns true if a new node is created
    int visit_node(const std::string& key, Element element, bool enter);
    void remove_node(int node);

    void calculate_sizes_up();
    void calculate_sizes_down();
    void render_tree();

    void mouse_button_callback(int button, int action, int mods);
    void key_callback(int key, int scancode, int action, int mods);

    static std::vector<std::pair<GLFWwindow*, Window*>> active_windows;
    static void glfw_mouse_button_callback(GLFWwindow* callback_window, int button, int action, int mods);
    static void glfw_key_callback(GLFWwindow* callback_window, int key, int scancode, int action, int mods);

    struct Events {
        bool mouse_down;
        bool mouse_up;
        bool key_down;
        bool key_up;
        int key;
        int mods;
        Events():
            mouse_down(false),
            mouse_up(false),
            key_down(false),
            key_up(false),
            key(-1),
            mods(0)
        {}
        void clear() {
            mouse_down = false;
            mouse_up = false;
            key_down = false;
            key_up = false;
            key = -1;
            mods = 0;
        }
    };
    Events events;

    struct {
        VectorMap<VerticalLayout> vertical_layout;
        VectorMap<HorizontalLayout> horizontal_layout;
        VectorMap<Text> text;
        VectorMap<Button> button;
        VectorMap<Checkbox> checkbox;
        VectorMap<TextInput> text_input;
    } elements;

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

    const Config config;
    const Style style;
    GLFWwindow* window;

    GeometryRenderer geometry_renderer;
    TextRenderer text_renderer;

    VectorMap<Node> nodes;
    int root_node;
    std::stack<int> active_nodes;
    int max_depth;
    int iteration;

    int node_pressed;
    int node_focused;

    TextStructure cursor_text;
    CursorPos cursor_begin;
    CursorPos cursor_end;
};

} // namespace datagui
