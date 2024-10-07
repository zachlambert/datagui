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

class LinearLayout {
public:
    LinearLayout& padding(float value) {
        padding_ = value;
        return *this;
    }

    LinearLayout& bg_color(const Color& value) {
        bg_color_ = value;
        return *this;
    }

    LinearLayout(const Vecf& input_size, bool horizontal):
        input_size_(input_size),
        horizontal_(horizontal),
        padding_(0),
        bg_color_(Color::Gray(0.5))
    {}

private:
    Vecf input_size_;
    bool horizontal_;
    float padding_;
    Color bg_color_;

    friend class ::datagui::Window;
};

class Text {
public:
    Text& max_width(float value) {
        max_width_ = value;
        return *this;
    }

    Text& line_height_factor(float value) {
        line_height_factor_ = value;
        return *this;
    }

    Text& bg_color(const Color& value) {
        bg_color_ = value;
        return *this;
    }

    Text& text_color(const Color& value) {
        text_color_ = value;
        return *this;
    }

    Text(const std::string& text):
        text_(text),
        max_width_(0),
        line_height_factor_(1),
        bg_color_(Color::White()),
        text_color_(Color::Black())
    {}

private:
    std::string text_;
    float max_width_;
    float line_height_factor_;
    Color bg_color_;
    Color text_color_;

    friend class ::datagui::Window;
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

    element::LinearLayout& linear_layout(bool horizontal, float width=0, float height=0) {
        active_node = create_node(Element::LinearLayout, elements.linear_layout.size(), active_node);
        depth++;
        max_depth = std::max(max_depth, depth);
        elements.linear_layout.emplace_back(Vecf(width, height), horizontal);
        return elements.linear_layout.back();
    }

    void layout_end() {
        if (active_node == -1) {
            throw std::runtime_error("Called end too many times");
        }
        active_node = nodes[active_node].parent;
        depth--;
    }

    element::Text& text(const std::string& text) {
        create_node(Element::Text, elements.text.size(), active_node);
        elements.text.emplace_back(text);
        return elements.text.back();
    }

private:
    void calculate_sizes_up();
    void calculate_sizes_down();
    void render_tree();

    enum class Element {
        None,
        LinearLayout,
        Text
    };

    int create_node(Element element, int element_index, int parent) {
        int node_index = nodes.size();

        Node node;
        node.element = element;
        node.element_index = element_index;
        node.parent = parent;

        if (parent != -1) {
            if (nodes[parent].first_child == -1) {
                nodes[parent].first_child = node_index;
            } else {
                nodes[nodes[parent].last_child].next = node_index;
            }
            nodes[parent].last_child = node_index;
        }

        nodes.push_back(node);
        return node_index;
    }

    struct {
        std::vector<element::LinearLayout> linear_layout;
        std::vector<element::Text> text;
    } elements;

    struct Node {
        // Definition
        Element element;
        int element_index;

        // Connectivity
        int parent;
        int next;
        int first_child;
        int last_child;

        // Layout calculation
        Vecf fixed_size;
        Vecf dynamic_size;
        Vecf origin;
        Vecf size;

        Node():
            element(Element::None),
            element_index(-1),
            parent(-1),
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
