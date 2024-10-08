#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <variant>
#include "datagui/font.hpp"
#include "datagui/geometry.hpp"
#include "datagui/geometry_renderer.hpp"
#include "datagui/text_renderer.hpp"


namespace datagui {


struct WindowInput {
    enum class Axis {
        LEFT_X,
        LEFT_Y,
        RIGHT_X,
        RIGHT_Y,
        COUNT
    };
    enum class Button {
        A,
        B,
        X,
        Y,
        LB,
        RB,
        COUNT
    };

    bool valid;
    std::array<double, (std::size_t)Axis::COUNT> axes;
    std::array<bool, (std::size_t)Button::COUNT> buttons;

    WindowInput():
        valid(false)
    {}
};

class Window;

namespace element {

struct VerticalLayout {
    struct Props {
        float padding = 0;
        Color bg_color = Color::White();
    };

    Vecf input_size;
    Props props;
    VerticalLayout(
        const Vecf& input_size,
        const Props& props
    ):
        input_size(input_size),
        props(props)
    {}
};

struct HorizontalLayout {
    struct Props {
        float padding = 0;
        Color bg_color = Color::White();
    };

    Vecf input_size;
    Props props;
    HorizontalLayout(
        const Vecf& input_size,
        const Props& props
    ):
        input_size(input_size),
        props(props)
    {}
};

struct Text {
    struct Props {
        float line_height_factor = 1;
        Color bg_color = Color::White();
        Color text_color = Color::Black();
    };

    std::string text;
    float max_width;
    Props props;
    Text(
        const std::string& text,
        float max_width,
        const Props& props
    ):
        text(text),
        max_width(max_width),
        props(props)
    {}
};

} // namespace element

class Window {
public:
    class Error: public std::runtime_error {
    public:
        Error(const std::string& message):
            std::runtime_error(message)
        {}
    };

    struct Config {
        Font font;
        int font_size;
        int width;
        int height;
        bool vsync;

        Config():
            font(Font::DejaVuSans),
            font_size(0),
            width(0),
            height(0),
            vsync(false)
        {}
        static Config defaults() {
            Config config;
            config.font = Font::DejaVuSans;
            config.font_size = 28;
            config.width = 900;
            config.height = 600;
            config.vsync = true;
            return config;
        }
    };

    Window(const std::string& title, const Config& config = Config::defaults(), bool open_now = true):
        title(title),
        config(config),
        window(nullptr),
        active_node(-1),
        depth(0),
        max_depth(0)
    {
        if (open_now) {
            open();
        }
    }
    ~Window() {
        if (window) {
            close();
        }
    }

    bool running() const {
        return window && !glfwWindowShouldClose(window);
    }

    void open();
    void poll_events();
    void render_begin();
    void render_end();
    void close();

    void vertical_layout(
        float width=0,
        float height=0,
        const element::VerticalLayout::Props& props = element::VerticalLayout::Props())
    {
        active_node = create_node(Element::VerticalLayout, elements.vertical_layout.size(), active_node);
        depth++;
        max_depth = std::max(max_depth, depth);
        elements.vertical_layout.emplace_back(Vecf(width, height), props);
    }

    void horizontal_layout(
        float width=0,
        float height=0,
        const element::HorizontalLayout::Props& props = element::HorizontalLayout::Props())
    {
        active_node = create_node(Element::HorizontalLayout, elements.horizontal_layout.size(), active_node);
        depth++;
        max_depth = std::max(max_depth, depth);
        elements.horizontal_layout.emplace_back(Vecf(width, height), props);
    }

    void layout_end() {
        if (active_node == -1) {
            throw std::runtime_error("Called end too many times");
        }
        active_node = nodes[active_node].parent;
        depth--;
    }

    element::Text& text(
        const std::string& text,
        float max_width = 0,
        const element::Text::Props& props = element::Text::Props())
    {
        create_node(Element::Text, elements.text.size(), active_node);
        elements.text.emplace_back(text, max_width, props);
        return elements.text.back();
    }

private:
    void calculate_sizes_up();
    void calculate_sizes_down();
    void render_tree();

    enum class Element {
        VerticalLayout,
        HorizontalLayout,
        Text
    };

    int create_node(Element element, int element_index, int parent) {
        int node_index = nodes.size();
        if (parent != -1) {
            if (nodes[parent].first_child == -1) {
                nodes[parent].first_child = node_index;
            } else {
                nodes[nodes[parent].last_child].next = node_index;
            }
            nodes[parent].last_child = node_index;
        }

        nodes.emplace_back(element, element_index, parent);
        return node_index;
    }

    struct {
        std::vector<element::VerticalLayout> vertical_layout;
        std::vector<element::HorizontalLayout> horizontal_layout;
        std::vector<element::Text> text;
    } elements;

    struct Node {
        // Definition
        const Element element;
        const int element_index;

        // Connectivity
        const int parent;
        int next;
        int first_child;
        int last_child;

        // Layout calculation
        Vecf fixed_size;
        Vecf dynamic_size;
        Vecf origin;
        Vecf size;

        Node(Element element, int element_index, int parent):
            element(element),
            element_index(element_index),
            parent(parent),
            next(-1),
            first_child(-1),
            last_child(-1),
            fixed_size(Vecf::Zero()),
            dynamic_size(Vecf::Zero()),
            origin(Vecf::Zero()),
            size(Vecf::Zero())
        {}
    };

    const std::string title;
    const Config config;
    GLFWwindow* window;
    WindowInput input;

    GeometryRenderer geometry_renderer;
    TextRenderer text_renderer;

    std::vector<Node> nodes;
    int active_node;
    int depth;
    int max_depth;
};

} // namespace datagui
