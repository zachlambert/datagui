#include "datagui/window.hpp"

#include <stack>
#include <iostream>
#include "datagui/exception.hpp"


namespace datagui {

std::vector<std::pair<GLFWwindow*, Window*>> Window::active_windows;

void Window::glfw_mouse_button_callback(GLFWwindow* callback_window, int button, int action, int mods) {
    for (auto [glfw_window, datagui_window]: active_windows) {
        if (glfw_window == callback_window) {
            datagui_window->mouse_button_callback(button, action, mods);
        }
    }
}

void Window::glfw_key_callback(GLFWwindow* callback_window, int key, int scancode, int action, int mods) {
    for (auto [glfw_window, datagui_window]: active_windows) {
        if (glfw_window == callback_window) {
            datagui_window->key_callback(key, scancode, action, mods);
        }
    }
}

Window::Window(const Config& config, const Style& style):
    config(config),
    style(style),
    window(nullptr),
    tree(std::bind(&Window::delete_element, this, std::placeholders::_1, std::placeholders::_2)),
    node_pressed(-1),
    node_focused(-1),
    node_clicked(-1)
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

    active_windows.emplace_back(window, this);
    glfwSetMouseButtonCallback(window, Window::glfw_mouse_button_callback);
    glfwSetKeyCallback(window, Window::glfw_key_callback);
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
    return node == node_clicked;
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

    vertical_layout("root", 0, 0);
}

void Window::render_end() {
    layout_end();
    if (tree.depth() != 0) {
        throw WindowError("Didn't call layout_... and layout_end the same number of times");
    }
    if (tree.root_node() == -1) {
        throw WindowError("Root node not defined");
    }

    calculate_sizes_up();
    calculate_sizes_down();
    event_handling();
    render_tree();

    const auto& root_node = tree[tree.root_node()];
    glfwSetWindowSizeLimits(window, root_node.fixed_size.x, root_node.fixed_size.y, -1, -1);

    glfwSwapBuffers(window);

    tree.reset();
}

void Window::calculate_sizes_up() {
    /*
      - Traverse down the tree, where if non-leaf node is reached, all the
         child tree are processed first.
      - Each node must calculate it's 'fixed_size' and 'dynamic_size', where:
        - For leaf tree, these are defined by the element and it's properties
        - For branch tree, these are defined by the element type, it's properties
          and the fixed_size/dynamic_size of the children
    */

    struct State {
        std::size_t index;
        bool first_visit;
        State(int index):
            index(index),
            first_visit(true)
        {}
    };
    std::stack<State> stack;
    stack.emplace(tree.root_node());

    while (!stack.empty()) {
        State& state = stack.top();
        Node& node = tree[state.index];

        // If the node has children, process these first
        if (node.first_child != -1 && state.first_visit) {
            state.first_visit = false;
            int child = node.first_child;
            while (child != -1) {
                stack.emplace(child);
                child = tree[child].next;
            }
            continue;
        }
        stack.pop();

        node.fixed_size = Vecf::Zero();
        node.dynamic_size = Vecf::Zero();

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
    }
}

void Window::calculate_sizes_down() {
    std::stack<int> stack;

    stack.push(tree.root_node());
    tree[tree.root_node()].size = window_size;

    while (!stack.empty()) {
        const auto& node = tree[stack.top()];
        stack.pop();

        if (node.first_child == -1) {
            continue;
        }

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

        int child = node.first_child;
        while (child != -1) {
            stack.push(child);
            child = tree[child].next;
        }
    }
}

void Window::mouse_button_callback(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            events.mouse_down = true;
        }
        if (action == GLFW_RELEASE){
            events.mouse_up = true;
        }
    }
}

void Window::key_callback(int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        events.key_down = true;
    }
    if (action == GLFW_RELEASE) {
        events.key_up = true;
    }
    events.key = key;
    events.mods = mods;
}

void Window::event_handling() {
    glfwPollEvents();

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    Vecf mouse_pos(mx, my);

    node_clicked = -1;

    if (events.mouse_down) {
        int clicked = tree.root_node();
        while (true) {
            const auto& node = tree[clicked];
            int child_index = node.first_child;
            while (child_index != -1) {
                const auto& child = tree[child_index];
                if (Boxf(child.origin, child.origin+child.size).contains(mouse_pos)) {
                    clicked = child_index;
                    break;
                }
                child_index = child.next;
            }
            if (child_index == -1) {
                break;
            }
        }

        node_pressed = clicked;
        node_focused = clicked;

        const auto& node = tree[clicked];
        switch (tree[clicked].element) {
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
                }
                break;
            default:
                break;
        }
    }

    if (events.mouse_up && node_pressed != -1) {
        const auto& node = tree[node_pressed];
        if (Boxf(node.origin, node.origin+node.size).contains(mouse_pos)) {
            node_clicked = node_pressed;
            // Handle click release
            switch (node.element) {
                case Element::Checkbox:
                    {
                        auto& element = elements.checkbox[node.element_index];
                        element.checked = !element.checked;
                    }
                    break;
                default:
                    break;
            }
        }
        node_pressed = -1;
    }

    if (node_pressed != -1) {
        const auto& node = tree[node_pressed];
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
                }
                break;
            default:
                break;
        }
    }

    if (node_pressed != -1 && events.key_down) {
        const auto& node = tree[node_focused];
        switch (node.element) {
            case Element::TextInput:
                {
                    auto& element = elements.text_input[node.element_index];
                    if (events.key == GLFW_KEY_LEFT || events.key == GLFW_KEY_RIGHT) {
                        if (cursor_begin.index != cursor_end.index && events.mods != 1) {
                            if (events.key == GLFW_KEY_LEFT && cursor_begin.index <= cursor_end.index
                                || events.key == GLFW_KEY_RIGHT && cursor_end.index <= cursor_begin.index)
                            {
                                cursor_end = cursor_begin;
                            } else {
                                cursor_begin = cursor_end;
                            }
                        } else {
                            std::size_t pos = cursor_end.index;
                            if (events.key == GLFW_KEY_LEFT && pos != 0) {
                                cursor_end.index = pos-1;
                                cursor_end.offset = text_renderer.find_cursor_offset(
                                    element.text, cursor_text, cursor_end.index);
                                if (events.mods != 1) {
                                    cursor_begin = cursor_end;
                                }
                            } else if (pos != element.text.size()) {
                                cursor_end.index = pos+1;
                                cursor_end.offset = text_renderer.find_cursor_offset(
                                    element.text, cursor_text, cursor_end.index);
                                if (events.mods != 1) {
                                    cursor_begin = cursor_end;
                                }
                            }
                        }
                    }
                    else if (events.key == GLFW_KEY_BACKSPACE) {
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
                    }
                    else if (events.key >= GLFW_KEY_A && events.key <= GLFW_KEY_Z) {
                        char new_c;
                        if (events.mods == 1) {
                            new_c = 'A' + (events.key - GLFW_KEY_A);
                        } else {
                            new_c = 'a' + (events.key - GLFW_KEY_A);
                        }
                        if (cursor_begin.index != cursor_end.index) {
                            std::size_t from = cursor_begin.index;
                            std::size_t to = cursor_end.index;
                            if (from > to) {
                                std::swap(from, to);
                            }
                            element.text.erase(element.text.begin() + from, element.text.begin() + to);
                            element.text.insert(element.text.begin() + from, new_c);
                            cursor_begin.index++;
                            cursor_text = text_renderer.calculate_text_structure(
                                element.text,
                                node.size.x - (style.element.border_width + style.element.padding),
                                style.text.line_height);
                            cursor_begin.offset = text_renderer.find_cursor_offset(
                                element.text, cursor_text, cursor_begin.index);
                            cursor_end = cursor_begin;
                        } else {
                            element.text.insert(element.text.begin() + cursor_begin.index, new_c);
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
                break;
            default:
                break;
        }
    }

    events.clear();
}

void Window::render_tree() {
    struct State {
        int node;
        int depth;
        State(int node, int depth):
            node(node),
            depth(depth)
        {}
    };

    std::stack<State> stack;
    stack.emplace(tree.root_node(), 0);

    while (!stack.empty()) {
        State state = stack.top();
        auto& node = tree[state.node];

        float normalized_depth = float(state.depth) / (tree.max_depth() + 1);
        stack.pop();

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
            }
            break;
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
            }
            break;
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
            }
            break;
        case Element::Button:
            {
                const auto& element = elements.button[node.element_index];
                const Color& bg_color =
                    (node_pressed == state.node)
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
            }
            break;
        case Element::Checkbox:
            {
                const auto& element = elements.checkbox[node.element_index];
                const Color& bg_color =
                    (node_pressed == state.node)
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
            }
            break;
        case Element::TextInput:
            {
                const auto& element = elements.text_input[node.element_index];
                geometry_renderer.queue_box(
                    normalized_depth,
                    Boxf(node.origin, node.origin+node.size),
                    style.element.bg_color,
                    style.element.border_width,
                    state.node == node_focused
                        ? style.element.focus_color
                        : style.element.border_color
                );
                Vecf text_origin =
                    node.origin
                    + Vecf::Constant(
                        style.element.border_width + style.element.padding);

                if (state.node == node_focused) {
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
            }
            break;
        }

        if (node.first_child == -1) {
            continue;
        }
        int child = node.first_child;
        while (child != -1) {
            stack.emplace(child, state.depth + 1);
            child = tree[child].next;
        }
    }

    geometry_renderer.render(window_size);
    text_renderer.render(window_size);
}

} // namespace datagui
