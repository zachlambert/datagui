#pragma once

#include <array>
#include <memory>
#include <stack>
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

// TODO:
// Currently requires everything is default-constructable and assumes
// that the memory cost for retaining popped nodes (until overwritten) is
// unimportant.
template <typename T>
class VectorMap {
public:
    const T& operator[](std::size_t i) const {
        return data[i];
    }
    T& operator[](std::size_t i) {
        return data[i];
    }
    template <typename ...Args>
    int emplace(Args&&... args) {
        int index = 0;
        if (free.empty()) {
            index = data.size();
            data.emplace_back(std::forward<Args>(args)...);
        } else {
            index = free.back();
            free.pop_back();
            data[index] = T(std::forward<Args>(args)...);
        }
        return index;
    }

    void pop(int index) {
        free.push_back(index);
    }

    std::size_t size() const {
        return data.size() - free.size();
    }

private:
    std::vector<T> data;
    std::vector<int> free;
};

class Window;

namespace element {

struct VerticalLayout {
    struct Props {
        float padding = 0;
        Color bg_color = Color::Clear();
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
        Color bg_color = Color::Clear();
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
        Color bg_color = Color::Clear();
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

struct Button {
    struct Props {
        float padding = 6;
        float border_width = 4;
        float line_height_factor = 1;
        Color bg_color = Color::White();
        Color text_color = Color::Black();
        Color border_color = Color::Black();
    };

    std::string text;
    float max_width;
    Props props;
    Button(
        const std::string& text,
        float max_width,
        const Props& props
    ):
        text(text),
        max_width(max_width),
        props(props)
    {}
};

struct Checkbox {
    struct Props {
        float size_factor = 1;
        float border_width = 4;
        Color border_color = Color::Black();
        Color bg_color = Color::Gray(0.5);
        Color icon_color = Color(0, 1, 1);
    };

    Props props;
    bool checked;

    Checkbox(
        const Props& props,
        bool default_checked = false
    ):
        props(props),
        checked(default_checked)
    {}
};

} // namespace element

class Window {
    enum class Element {
        VerticalLayout,
        HorizontalLayout,
        Text,
        Button,
        Checkbox
    };

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
        root_node(-1),
        max_depth(0),
        iteration(0),
        node_pressed(-1)
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
    void render_begin();
    void render_end();
    void close();

    void vertical_layout(
        const std::string& key,
        float width=0,
        float height=0,
        const element::VerticalLayout::Props& props = element::VerticalLayout::Props());

    void horizontal_layout(
        const std::string& key,
        float width=0,
        float height=0,
        const element::HorizontalLayout::Props& props = element::HorizontalLayout::Props());

    void layout_end();

    void text(
        const std::string& key,
        const std::string& text,
        float max_width = 0,
        const element::Text::Props& props = element::Text::Props());

    bool button(
        const std::string& key,
        const std::string& text,
        float max_width = 0,
        const element::Button::Props& props = element::Button::Props());

    bool checkbox(
        const std::string& key,
        const element::Checkbox::Props& props = element::Checkbox::Props());

private:
    // Returns true if a new node is created
    int visit_node(const std::string& key, Element element, bool enter);
    void remove_node(int node);

    void calculate_sizes_up();
    void calculate_sizes_down();
    void render_tree();

    void mouse_button_callback(int button, int action, int mods);
    static void glfw_mouse_button_callback(GLFWwindow* callback_window, int button, int action, int mods);

    static std::vector<std::pair<GLFWwindow*, Window*>> active_windows;

    struct Events {
        bool mouse_down;
        bool mouse_up;
        Events():
            mouse_down(false),
            mouse_up(false)
        {}
        void clear() {
            mouse_down = false;
            mouse_up = false;
        }
    };
    Events events;

    struct {
        VectorMap<element::VerticalLayout> vertical_layout;
        VectorMap<element::HorizontalLayout> horizontal_layout;
        VectorMap<element::Text> text;
        VectorMap<element::Button> button;
        VectorMap<element::Checkbox> checkbox;
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

    const std::string title;
    const Config config;
    GLFWwindow* window;
    WindowInput input;

    GeometryRenderer geometry_renderer;
    TextRenderer text_renderer;

    VectorMap<Node> nodes;
    int root_node;
    std::stack<int> active_nodes;
    int max_depth;
    int iteration;

    int node_pressed;
};

} // namespace datagui
