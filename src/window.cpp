#include "datagui/window.hpp"

#include <GLFW/glfw3.h>
#include <stack>
#include <iostream>
#include <cstring>
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
    tree(std::bind(&Window::delete_element, this, std::placeholders::_1, std::placeholders::_2))
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);

    geometry_renderer.init();
    text_renderer.init(style.text);

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
            elements.button.pop(element_index);
            break;
        case Element::Checkbox:
            elements.checkbox.pop(element_index);
            break;
        case Element::TextInput:
            elements.text_input.pop(element_index);
            break;
    }
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
        return elements.button.emplace(text, max_width);
    });
    tree.up();
    return node == tree.node_released();
}

bool Window::checkbox(const std::string& key) {
    int node = tree.down(key, Element::Checkbox, [&](){
        return elements.checkbox.emplace();
    });
    tree.up();
    return elements.checkbox[tree[node].element_index].checked;
}

bool Window::text_input(
    const std::string& key,
    const std::string& default_text,
    float max_width)
{
    int node = tree.down(key, Element::TextInput, [&](){
        return elements.text_input.emplace(default_text, max_width);
    });
    tree.up();
    // TODO: changed field isn't updated at the moment
    return elements.text_input[tree[node].element_index].changed;
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
    vertical_layout("root", 0, 0);
}

void Window::render_end() {
    layout_end();
    tree.end(
        window_size,
        [&](Node& node) {
            switch (node.element) {
                case Element::VerticalLayout:
                    calculate_size_components(
                        tree, style,
                        node, elements.vertical_layout[node.element_index]);
                    break;
                case Element::HorizontalLayout:
                    calculate_size_components(
                        tree, style,
                        node, elements.horizontal_layout[node.element_index]);
                    break;
                case Element::Text:
                    calculate_size_components(
                        tree, style, text_renderer,
                        node, elements.text[node.element_index]);
                    break;
                case Element::Button:
                    calculate_size_components(
                        tree, style, text_renderer,
                        node, elements.button[node.element_index]);
                    break;
                case Element::Checkbox:
                    calculate_size_components(
                        tree, style,
                        node, elements.checkbox[node.element_index]);
                    break;
                case Element::TextInput:
                    calculate_size_components(
                        tree, style, text_renderer,
                        node, elements.text_input[node.element_index]);
                    break;
            }
        },
        [&](const Node& node) {
            switch (node.element) {
                case Element::VerticalLayout:
                    calculate_child_dimensions(
                        tree, style,
                        node, elements.vertical_layout[node.element_index]);
                    break;
                case Element::HorizontalLayout:
                    calculate_child_dimensions(
                        tree, style,
                        node, elements.horizontal_layout[node.element_index]);
                    break;
                default:
                    throw WindowError("A non-layout element shouldn't have children");
                    break;
            }
        }
    );

    event_handling();
    render_tree();

    const auto& root_node = tree[tree.root_node()];
    glfwSetWindowSizeLimits(window, root_node.fixed_size.x, root_node.fixed_size.y, -1, -1);

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
                auto& element = elements.text_input[node.element_index];
                cursor_text = text_renderer.calculate_text_structure(
                    element.text,
                    node.size.x - (style.element.border_width + style.element.padding),
                    style.text.line_height
                );
                cursor_begin = text_renderer.find_cursor(
                    element.text,
                    cursor_text,
                    node.origin + Vecf::Constant(
                        style.element.border_width + style.element.padding),
                    mouse_pos
                );
                cursor_end = cursor_begin;
                break;
            }
            default:
                 break;
        }
    }

    if (tree.node_released()) {
        const auto& node = tree[tree.node_released()];
        switch (node.element) {
            case Element::Checkbox:
            {
                auto& element = elements.checkbox[node.element_index];
                element.checked = !element.checked;
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
                auto& element = elements.text_input[node.element_index];
                cursor_end = text_renderer.find_cursor(
                    element.text,
                    cursor_text,
                    node.origin + Vecf::Constant(
                        style.element.border_width + style.element.padding),
                    mouse_pos
                );
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
                            auto& element = elements.text_input[new_node.element_index];
                            cursor_text = text_renderer.calculate_text_structure(
                                element.text,
                                new_node.size.x - (style.element.border_width + style.element.padding),
                                style.text.line_height
                            );
                            cursor_begin.index = 0;
                            cursor_begin.offset = text_renderer.find_cursor_offset(
                                element.text,
                                cursor_text,
                                cursor_begin.index
                            );
                            cursor_end = cursor_begin;
                            break;
                        }
                        default:
                            break;
                    }
                }
                break;
            case GLFW_KEY_ESCAPE:
                tree.focus_escape();
                break;
        }
    }

    if (tree.node_focused() != -1 && tree[tree.node_focused()].element == Element::TextInput) {
        const auto& node = tree[tree.node_focused()];
        auto& element = elements.text_input[node.element_index];

        if (events.key.action == GLFW_PRESS || events.key.action == GLFW_REPEAT) {
            switch (events.key.key) {
                case GLFW_KEY_LEFT:
                {
                    if (cursor_begin.index != cursor_end.index && events.key.mods != 1) {
                        if (cursor_begin.index <= cursor_end.index) {
                            cursor_end = cursor_begin;
                        } else {
                            cursor_begin = cursor_end;
                        }
                    } else if (cursor_end.index != 0) {
                        cursor_end.index--;
                        cursor_end.offset = text_renderer.find_cursor_offset(
                            element.text, cursor_text, cursor_end.index);
                        if (events.key.mods != 1) {
                            cursor_begin = cursor_end;
                        }
                    }
                    break;
                }
                case GLFW_KEY_RIGHT:
                {
                    if (cursor_begin.index != cursor_end.index && events.key.mods != 1) {
                        if (cursor_begin.index <= cursor_end.index) {
                            cursor_begin = cursor_end;
                        } else {
                            cursor_end = cursor_begin;
                        }
                    } else if (cursor_end.index != element.text.size()) {
                        cursor_end.index++;
                        cursor_end.offset = text_renderer.find_cursor_offset(
                            element.text, cursor_text, cursor_end.index);
                        if (events.key.mods != 1) {
                            cursor_begin = cursor_end;
                        }
                    }
                    break;
                }
                case GLFW_KEY_BACKSPACE:
                {
                    if (cursor_begin.index != cursor_end.index) {
                        std::size_t from = cursor_begin.index;
                        std::size_t to = cursor_end.index;
                        if (from > to) {
                            std::swap(from, to);
                        }
                        element.text.erase(element.text.begin() + from, element.text.begin() + to);
                        if (cursor_begin.index <= cursor_end.index) {
                            cursor_end = cursor_begin;
                        } else {
                            cursor_begin = cursor_end;
                        }
                        cursor_text = text_renderer.calculate_text_structure(
                            element.text,
                            node.size.x - (style.element.border_width + style.element.padding),
                            style.text.line_height);
                    } else if (cursor_begin.index > 0) {
                        element.text.erase(element.text.begin() + (cursor_begin.index-1));
                        cursor_text = text_renderer.calculate_text_structure(
                            element.text,
                            node.size.x - (style.element.border_width + style.element.padding),
                            style.text.line_height);
                        cursor_begin.index--;
                        cursor_begin.offset = text_renderer.find_cursor_offset(
                            element.text, cursor_text, cursor_begin.index);
                        cursor_end = cursor_begin;
                    }
                    break;
                }
            }
        }

        if (events.text.received) {
            if (cursor_begin.index != cursor_end.index) {
                std::size_t from = cursor_begin.index;
                std::size_t to = cursor_end.index;
                if (from > to) {
                    std::swap(from, to);
                }
                element.text.erase(element.text.begin() + from, element.text.begin() + to);
                element.text.insert(element.text.begin() + from, events.text.character);
                cursor_begin.index++;
                cursor_text = text_renderer.calculate_text_structure(
                    element.text,
                    node.size.x - (style.element.border_width + style.element.padding),
                    style.text.line_height);
                cursor_begin.offset = text_renderer.find_cursor_offset(
                    element.text, cursor_text, cursor_begin.index);
                cursor_end = cursor_begin;
            } else {
                element.text.insert(element.text.begin() + cursor_begin.index, events.text.character);
                cursor_begin.index++;
                cursor_text = text_renderer.calculate_text_structure(
                    element.text,
                    node.size.x - (style.element.border_width + style.element.padding),
                    style.text.line_height);
                cursor_begin.offset = text_renderer.find_cursor_offset(
                    element.text, cursor_text, cursor_begin.index);
                cursor_end = cursor_begin;
            }
        }
    }

    events.reset();
}

void Window::render_tree() {
    std::stack<int> stack;
    stack.push(tree.root_node());

    int max_layer = tree.max_depth() + 1;

    while (!stack.empty()) {
        int node_index = stack.top();
        auto& node = tree[stack.top()];
        stack.pop();

        float normalized_depth = float(node.depth) / max_layer;

        switch (node.element) {
            case Element::VerticalLayout:
            {
                const auto& element = elements.vertical_layout[node.element_index];
                geometry_renderer.queue_box(
                    normalized_depth,
                    Boxf(node.origin, node.origin+node.size),
                    Color::Clear(),
                    style.element.border_width,
                    style.element.border_color,
                    0
                );
                break;
            }
            case Element::HorizontalLayout:
            {
                const auto& element = elements.horizontal_layout[node.element_index];
                geometry_renderer.queue_box(
                    normalized_depth,
                    Boxf(node.origin, node.origin+node.size),
                    Color::Clear(),
                    style.element.border_width,
                    style.element.border_color,
                    0
                );
                break;
            }
            case Element::Text:
            {
                const auto& element = elements.text[node.element_index];
                geometry_renderer.queue_box(
                    normalized_depth,
                    Boxf(node.origin, node.origin+node.size),
                    Color::Clear(),
                    0,
                    Color::Black(),
                    0
                );
                text_renderer.queue_text(
                    element.text,
                    element.max_width,
                    style.text.line_height,
                    node.origin,
                    normalized_depth,
                    style.text.font_color
                );
                break;
            }
            case Element::Button:
            {
                const auto& element = elements.button[node.element_index];
                const Color& bg_color =
                    (tree.node_held() == node_index)
                        ? style.element.pressed_bg_color
                        :style.element.bg_color;
                geometry_renderer.queue_box(
                    normalized_depth,
                    Boxf(node.origin, node.origin+node.size),
                    bg_color,
                    style.element.border_width,
                    style.element.border_color
                );
                text_renderer.queue_text(
                    element.text,
                    element.max_width,
                    style.text.line_height,
                    node.origin + Vecf::Constant(style.element.border_width + style.element.padding),
                    normalized_depth,
                    style.text.font_color
                );
                break;
            }
            case Element::Checkbox:
            {
                const auto& element = elements.checkbox[node.element_index];
                const Color& bg_color =
                    (tree.node_held() == node_index)
                    ? style.element.pressed_bg_color
                    : style.element.bg_color;

                geometry_renderer.queue_box(
                    normalized_depth,
                    Boxf(node.origin, node.origin+node.size),
                    bg_color,
                    style.element.border_width,
                    style.element.border_color,
                    0
                );

                if (element.checked) {
                    float offset = style.element.border_width + style.checkbox.check_padding;
                    geometry_renderer.queue_box(
                        normalized_depth,
                        Boxf(
                            node.origin + Vecf::Constant(offset),
                            node.origin + node.size - Vecf::Constant(offset)
                        ),
                        style.checkbox.check_color,
                        0,
                        Color::Black(),
                        0
                    );
                }
                break;
            }
            case Element::TextInput:
            {
                const auto& element = elements.text_input[node.element_index];
                geometry_renderer.queue_box(
                    normalized_depth,
                    Boxf(node.origin, node.origin+node.size),
                    style.element.bg_color,
                    style.element.border_width,
                    node_index == tree.node_focused()
                        ? style.element.focus_color
                        : style.element.border_color
                );
                Vecf text_origin =
                    node.origin
                    + Vecf::Constant(
                        style.element.border_width + style.element.padding);

                if (node_index == tree.node_focused()) {
                    if (cursor_begin.index == cursor_end.index) {
                        // TODO: Blink
                        geometry_renderer.queue_box(
                            normalized_depth,
                            Boxf(
                                text_origin+cursor_begin.offset-Vecf(float(style.text_input.cursor_width)/2, 0),
                                text_origin+cursor_begin.offset + Vecf(style.text_input.cursor_width, cursor_text.line_height)),
                            style.text_input.cursor_color,
                            0,
                            Color::Black(),
                            0
                        );
                    } else {
                        CursorPos from, to;
                        if (cursor_begin.index <= cursor_end.index) {
                            from = cursor_begin;
                            to = cursor_end;
                        } else {
                            from = cursor_end;
                            to = cursor_begin;
                        }

                        for (std::size_t line_i = 0; line_i < cursor_text.lines.size(); line_i++) {
                            const auto& line = cursor_text.lines[line_i];
                            if (from.index >= line.end) {
                                continue;
                            }
                            CursorPos line_to;
                            if (to.index > line.end) {
                                line_to = CursorPos{line.end, Vecf(line.width, line_i * cursor_text.line_height)};
                            } else {
                                line_to = to;
                            }
                            geometry_renderer.queue_box(
                                normalized_depth,
                                Boxf(
                                    text_origin + from.offset,
                                    text_origin + line_to.offset + Vecf(0, cursor_text.line_height)
                                ),
                                style.text_input.highlight_color,
                                0,
                                Color::Black(),
                                0
                            );
                            if (line_i != cursor_text.lines.size()-1) {
                                const auto& next_line = cursor_text.lines[line_i+1];
                                from.index = next_line.begin;
                                from.offset = Vecf(0, (line_i+1) * cursor_text.line_height);
                            }
                            if (to.index <= line.end) {
                                break;
                            }
                        }
                    }

                }
                text_renderer.queue_text(
                    element.text,
                    element.max_width,
                    style.text.line_height,
                    text_origin,
                    normalized_depth,
                    style.text.font_color
                );
                break;
            }
        }

        if (node.first_child == -1) {
            continue;
        }
        int child = node.first_child;
        while (child != -1) {
            stack.push(child);
            child = tree[child].next;
        }
    }

    geometry_renderer.render(window_size);
    text_renderer.render(window_size);
}

} // namespace datagui
