#include "datagui/window.hpp"

#include <GLFW/glfw3.h>
#include <stack>
#include <iostream>
#include <cstring>
#include <assert.h>
#include "datagui/exception.hpp"


namespace datagui {

struct Events {
    struct {
        int action = -1;
        int button = 0;
    } mouse;
    struct {
        int action = -1;
        int key = 0;
        int mods = 0;
    } key;
    struct {
        bool received = false;
        char character = 0;
    } text;

    void reset() {
        mouse.action = -1;
        key.action = -1;
        text.received = false;
    }
} events;

void glfw_mouse_button_callback(GLFWwindow* callback_window, int button, int action, int mods) {
    events.mouse.action = action;
    events.mouse.button = button;
}

void glfw_key_callback(GLFWwindow* callback_window, int key, int scancode, int action, int mods) {
    events.key.action = action;
    events.key.key = key;
    events.key.mods = mods;
}

void glfw_char_callback(GLFWwindow* callback_window, unsigned int codepoint) {
    if (codepoint < 256) {
        events.text.received = true;
        events.text.character = char(codepoint);
    }
}


Window::Window(const Config& config, const Style& style):
    config(config),
    style(style),
    window(nullptr),
    window_size(Vecf::Zero()),
    tree(std::bind(
        &Window::delete_element,
        this,
        std::placeholders::_1,
        std::placeholders::_2)),
    buttons(style, font, renderers.geometry, renderers.text)
{
    open();
}

Window::~Window() {
    if (window) {
        close();
    }
}

bool Window::running() const {
    return window && !glfwWindowShouldClose(window);
}

void Window::open() {
    if (!glfwInit()) {
        throw InitializationError("Failed to initialize glfw");
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    window = glfwCreateWindow(
        config.width,
        config.height,
        config.title.c_str(),
        nullptr,
        nullptr);

    if (window == nullptr) {
        throw InitializationError("Failed to create window");
    }

    glfwMakeContextCurrent(window);

    if (config.vsync) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }

    if (glewInit() != GLEW_OK) {
        throw InitializationError("Failed to initialise glew");
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    font = load_font(style.text.font, style.text.font_size);
    renderers.geometry.init();
    renderers.text.init();

    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetCharCallback(window, glfw_char_callback);
}

void Window::close() {
    if (!window) {
        throw WindowError("Window is already closed");
    }

    glfwMakeContextCurrent(window);
    glfwDestroyWindow(window);
    glfwTerminate();
    window = nullptr;
}

void Window::delete_element(Element element, int element_index) {
    switch (element) {
        case Element::VerticalLayout:
            elements.vertical_layout.pop(element_index);
            break;
        case Element::HorizontalLayout:
            elements.horizontal_layout.pop(element_index);
            break;
        case Element::Text:
            elements.text.pop(element_index);
            break;
        case Element::Button:
            buttons.pop(element_index);
            break;
        case Element::Checkbox:
            elements.checkbox.pop(element_index);
            break;
        case Element::TextInput:
            elements.text_input.pop(element_index);
            break;
    }
}

const ElementInterface& Window::get_element(const Node& node) {
    switch (node.element) {
        case Element::Button:
            assert(false);
            break;
        case Element::Checkbox:
            return elements.checkbox[node.element_index];
            break;
        case Element::HorizontalLayout:
            return elements.horizontal_layout[node.element_index];
            break;
        case Element::Text:
            return elements.text[node.element_index];
            break;
        case Element::TextInput:
            return elements.text_input[node.element_index];
            break;
        case Element::VerticalLayout:
            return elements.vertical_layout[node.element_index];
            break;
    }
    assert(false);
    throw WindowError("Unreachable code");
    return elements.checkbox[0];
}

ElementInterface& Window::get_mutable_element(const Node& node) {
    return const_cast<ElementInterface&>(get_element(node));
}

void Window::vertical_layout(
    const std::string& key,
    float width,
    float height)
{
    tree.down(key, Element::VerticalLayout, [&]() {
        return elements.vertical_layout.emplace(Vecf(width, height));
    });
}

void Window::horizontal_layout(
    const std::string& key,
    float width,
    float height)
{
    tree.down(key, Element::HorizontalLayout, [&]() {
        return elements.horizontal_layout.emplace(Vecf(width, height));
    });
}

void Window::layout_end() {
    tree.up();
}

void Window::text(
    const std::string& key,
    const std::string& text,
    float max_width)
{
    tree.down(key, Element::Text, [&](){
        return elements.text.emplace(text, max_width);
    });
    tree.up();
}

bool Window::button(
    const std::string& key,
    const std::string& text,
    float max_width)
{
    int node = tree.down(key, Element::Button, [&](){
        return buttons.create(text, max_width);
    });
    tree.up();
    return node == tree.node_released();
}

const bool* Window::checkbox(const std::string& key) {
    int node = tree.down(key, Element::Checkbox, [&](){
        return elements.checkbox.emplace(false);
    });
    tree.up();
    const auto& element = elements.checkbox[tree[node].element_index];
    if (tree.node_released() == node) {
        return &element.checked();
    }
    return nullptr;
}

const std::string* Window::text_input(
    const std::string& key,
    const std::string& default_text,
    float max_width)
{
    int node = tree.down(key, Element::TextInput, [&](){
        return elements.text_input.emplace(default_text, max_width);
    });
    tree.up();
    auto& element = elements.text_input[tree[node].element_index];
    if (element.check_changed()) {
        return &element.text();
    }
    return nullptr;
}

void Window::render_begin() {
    glClearColor(style.bg_color.r, style.bg_color.g, style.bg_color.b, 1);
    glClearDepth(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    window_size = Vecf(display_w, display_h);

    tree.begin();
}

void Window::render_end() {
    tree.end(
        window_size,
        [&](Node& node) {
            if (node.element == Element::Button) {
                buttons.calculate_size_components(node, tree);
                return;
            }
            get_element(node).calculate_size_components(style, font, node, tree);
        },
        [&](const Node& node) {
            get_element(node).calculate_child_dimensions(style, node, tree);
        }
    );

    event_handling();

    tree.render([&](const Node& node, const NodeState& state) {
        if (node.element == Element::Button) {
            buttons.render(node, state);
            return;
        }
        get_element(node).render(
            style,
            font,
            node,
            state,
            selection,
            renderers);
    });

    renderers.geometry.render(window_size);
    renderers.text.render(window_size);

    if (tree.root_node() != -1) {
        const auto& root_node = tree[tree.root_node()];
        glfwSetWindowSizeLimits(
            window,
            root_node.fixed_size.x, root_node.fixed_size.y,
            -1, -1);
    }

    glfwSwapBuffers(window);
}

void Window::event_handling() {
    glfwPollEvents();

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    Vecf mouse_pos(mx, my);

    tree.mouse_reset();
    if (events.mouse.action == GLFW_PRESS) {
        tree.mouse_press(mouse_pos);
    }
    if (events.mouse.action == GLFW_RELEASE) {
        tree.mouse_release(mouse_pos);
    }

    if (tree.node_pressed() != -1) {
        const auto& node = tree[tree.node_pressed()];
        switch (node.element) {
            case Element::TextInput:
            {
                const auto& element = elements.text_input[node.element_index];
                Vecf text_origin = node.origin + Vecf::Constant(
                    style.element.border_width + style.element.padding);
                selection.begin = find_cursor(
                    font,
                    element.text(),
                    element.max_width,
                    mouse_pos - text_origin
                );
                break;
            }
            case Element::Text:
            {
                const auto& element = elements.text[node.element_index];
                selection.begin = find_cursor(
                    font,
                    element.text,
                    element.max_width,
                    mouse_pos - node.origin
                );
                break;
            }
            default:
                 break;
        }
    }

    if (tree.node_released() != -1) {
        const auto& node = tree[tree.node_released()];
        switch (node.element) {
            case Element::Checkbox:
            {
                auto& element = elements.checkbox[node.element_index];
                element.toggle();
                break;
            }
            default:
                break;
        }
    }

    // Handle element-specific logic while pressed down
    if (tree.node_held() != -1) {
        const auto& node = tree[tree.node_held()];
        switch (node.element) {
            case Element::TextInput:
            {
                const auto& element = elements.text_input[node.element_index];
                Vecf text_origin = node.origin + Vecf::Constant(
                    style.element.border_width + style.element.padding);
                selection.end = find_cursor(
                    font,
                    element.text_,
                    element.max_width,
                    mouse_pos - text_origin);
                break;
            }
            case Element::Text:
            {
                const auto& element = elements.text[node.element_index];
                selection.end = find_cursor(
                    font,
                    element.text,
                    element.max_width,
                    mouse_pos - node.origin);
                break;
            }
            default:
                break;
        }
    }

    if (events.key.action == GLFW_PRESS || events.key.action == GLFW_REPEAT) {
        switch (events.key.key) {
            case GLFW_KEY_TAB:
                if (tree.focus_next()) {
                    const auto& new_node = tree[tree.node_focused()];
                    switch (new_node.element) {
                        case Element::TextInput:
                        {
                            const auto& element = elements.text_input[new_node.element_index];
                            selection.begin = 0;
                            selection.end = 0;
                            break;
                        }
                        default:
                            break;
                    }
                }
                break;
            case GLFW_KEY_ESCAPE:
                if (tree.node_focused() != -1) {
                    const auto& node = tree[tree.node_focused()];
                    switch (node.element) {
                        case Element::TextInput:
                        {
                            auto& element = elements.text_input[node.element_index];
                            tree.focus_escape(false);
                            element.revert();
                            break;
                        }
                        default:
                        break;
                    }
                }
                break;
            case GLFW_KEY_ENTER:
                if (tree.node_focused() != -1) {
                    tree.focus_escape(true);
                }
                break;
        }
    }

    if (tree.node_focus_released() != -1) {
        const auto& node = tree[tree.node_focus_released()];
        switch (node.element) {
            case Element::TextInput:
            {
                auto& element = elements.text_input[node.element_index];
                element.confirm();
                break;
            }
            default:
                break;
        }
    }

    if (tree.node_focused() != -1 && (events.key.action == GLFW_PRESS || events.key.action == GLFW_REPEAT)) {
        const auto& node = tree[tree.node_focused()];
        switch (node.element) {
            case Element::TextInput:
            {
                auto& element = elements.text_input[node.element_index];
                selection_input_key(element.text_, selection, events.key.key, events.key.mods, true);
                break;
            }
            case Element::Text:
            {
                auto& element = elements.text_input[node.element_index];
                selection_input_key(element.text_, selection, events.key.key, events.key.mods, false);
                break;
            }
            default:
                break;
        }
    }

    if (tree.node_focused() != -1 && events.text.received) {
        const auto& node = tree[tree.node_focused()];
        switch (node.element) {
            case Element::TextInput:
            {
                auto& element = elements.text_input[node.element_index];
                selection_input_char(element.text_, selection, events.text.character, true);
                break;
            }
            case Element::Text:
            {
                auto& element = elements.text_input[node.element_index];
                selection_input_char(element.text_, selection, events.text.character, false);
                break;
            }
            default:
                break;
        }
    }

    events.reset();
}

} // namespace datagui
