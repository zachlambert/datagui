#include "datagui/window.hpp"

#include <stack>
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
    node_focused(-1)
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
    return tree[node].clicked;
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
    glClearColor(0.5, 0.5, 0.5, 1);
    glClearDepth(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    vertical_layout("root", display_w, display_h);
}

void Window::render_end() {
    layout_end();
    if (tree.depth() != 0) {
        throw WindowError("Didn't call layout_... and layout_end the same number of times");
    }

    glfwPollEvents();

    calculate_sizes_up();
    calculate_sizes_down();
    render_tree();

    glfwSwapBuffers(window);

    events.clear();
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
    stack.emplace(0);

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

        switch (node.element) {
        case Element::VerticalLayout:
            {
                const auto& element = elements.vertical_layout[node.element_index];

                // X direction
                if (element.input_size.x == 0) {
                    int child = node.first_child;
                    int count = 0;
                    while (child != -1) {
                        count++;
                        node.fixed_size.x = std::max(node.fixed_size.x, tree[child].fixed_size.x);
                        node.dynamic_size.x = std::max(node.dynamic_size.x, tree[child].dynamic_size.x);
                        child = tree[child].next;
                    }
                    node.fixed_size.x += 2 * style.element.padding;

                } else if (element.input_size.x > 0) {
                    node.fixed_size.x = element.input_size.x;
                } else {
                    node.dynamic_size.x = -element.input_size.x;
                }

                // Y direction
                if (element.input_size.y == 0) {
                    int child = node.first_child;
                    int count = 0;
                    while (child != -1) {
                        count++;
                        node.fixed_size.y += tree[child].fixed_size.y;
                        node.dynamic_size.y += tree[child].dynamic_size.y;
                        child = tree[child].next;
                    }
                    node.fixed_size.y += 2 * style.element.padding;
                    node.fixed_size.y += (count - 1) * style.element.padding;

                } else if (element.input_size.y > 0) {
                    node.fixed_size.y = element.input_size.y;
                } else {
                    node.dynamic_size.y = -element.input_size.y;
                }
            }
            break;
        case Element::HorizontalLayout:
            {
                const auto& element = elements.horizontal_layout[node.element_index];

                // X direction
                if (element.input_size.x == 0) {
                    int child = node.first_child;
                    int count = 0;
                    while (child != -1) {
                        count++;
                        node.fixed_size.x += tree[child].fixed_size.x;
                        node.dynamic_size.x += tree[child].dynamic_size.x;
                        child = tree[child].next;
                    }
                    node.fixed_size.x += 2 * style.element.padding;
                    node.fixed_size.x += (count - 1) * style.element.padding;

                } else if (element.input_size.x > 0) {
                    node.fixed_size.x = element.input_size.x;
                } else {
                    node.dynamic_size.x = -element.input_size.x;
                }

                // Y direction
                if (element.input_size.y == 0) {
                    int child = node.first_child;
                    int count = 0;
                    while (child != -1) {
                        count++;
                        node.fixed_size.y = std::max(node.fixed_size.y, tree[child].fixed_size.y);
                        node.dynamic_size.y = std::max(node.dynamic_size.y, tree[child].dynamic_size.y);
                        child = tree[child].next;
                    }
                    node.fixed_size.y += 2 * style.element.padding;
                    node.fixed_size.y += (count - 1) * style.element.padding;

                } else if (element.input_size.y > 0) {
                    node.fixed_size.y = element.input_size.y;
                } else {
                    node.dynamic_size.y = -element.input_size.y;
                }
            }
            break;
        case Element::Text:
            {
                const auto& element = elements.text[node.element_index];

                node.fixed_size = text_renderer.text_size(
                    element.text,
                    element.max_width,
                    style.text.line_height);
            }
            break;
        case Element::Button:
            {
                const auto& element = elements.button[node.element_index];

                node.fixed_size = text_renderer.text_size(
                    element.text,
                    element.max_width,
                    style.text.line_height);
                node.fixed_size += Vecf::Constant((style.element.border_width + style.element.padding) * 2);
            }
            break;
        case Element::Checkbox:
            {
                const auto& element = elements.checkbox[node.element_index];
                node.fixed_size = Vecf::Constant(text_renderer.get_font_size() * style.checkbox.size);
            }
            break;
        case Element::TextInput:
            {
                const auto& element = elements.text_input[node.element_index];

                node.fixed_size = text_renderer.text_size(
                    element.text,
                    element.max_width,
                    style.text.line_height);
                node.fixed_size += Vecf::Constant(
                    2 * (style.element.border_width + style.element.padding));
            }
            break;
        };
    }
}

void Window::calculate_sizes_down() {
    std::stack<int> stack;
    stack.push(0);

    tree[0].size = tree[0].fixed_size;
    tree[0].origin = Vecf::Zero();

    while (!stack.empty()) {
        const auto& parent = tree[stack.top()];
        stack.pop();
        if (parent.first_child == -1) {
            continue;
        }

        Vecf available = parent.size - parent.fixed_size;
        Vecf offset = Vecf::Zero();

        switch (parent.element) {
            case Element::VerticalLayout:
                {
                    const auto& element = elements.vertical_layout[parent.element_index];
                    offset.x += style.element.padding;
                    offset.y += style.element.padding;
                }
                break;
            case Element::HorizontalLayout:
                {
                    const auto& element = elements.horizontal_layout[parent.element_index];
                    offset.x += style.element.padding;
                    offset.y += style.element.padding;
                }
                break;
            default:
                break;
        }

        int child = parent.first_child;
        while (child != -1) {
            auto& child_node = tree[child];
            child_node.size = child_node.fixed_size;
            if (parent.dynamic_size.x > 0){
                child_node.size.x += child_node.dynamic_size.x / parent.dynamic_size.x;
            }
            if (parent.dynamic_size.y > 0){
                child_node.size.y += child_node.dynamic_size.y / parent.dynamic_size.y;
            }
            child_node.origin = parent.origin + offset;

            switch (parent.element) {
            case Element::VerticalLayout:
                {
                    const auto& element = elements.vertical_layout[parent.element_index];
                    offset.y += child_node.size.y + style.element.padding;
                }
                break;
            case Element::HorizontalLayout:
                {
                    const auto& element = elements.horizontal_layout[parent.element_index];
                    offset.x += child_node.size.x + style.element.padding;
                }
                break;
            default:
                throw std::runtime_error("A non-layout element shouldn't have children");
                break;
            }

            stack.push(child);
            child = child_node.next;
        }
    }
}

void Window::render_tree() {
    struct State {
        int node;
        bool parent_clicked;
        int depth;
        State(int node, bool parent_clicked, int depth):
            node(node),
            parent_clicked(parent_clicked),
            depth(depth)
        {}
    };
    std::stack<State> stack;
    stack.emplace(tree.root_node(), events.mouse_up || events.mouse_down, 0);

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    Vecf mouse_pos(mx, my);

    int node_clicked = -1;
    int node_clicked_depth = -1;

    while (!stack.empty()) {
        State state = stack.top();
        auto& node = tree[state.node];

        // Handle element-specific click events
        if (node.clicked) {
            switch (node.element) {
                case Element::Checkbox:
                    {
                        auto& element = elements.checkbox[node.element_index];
                        element.checked = !element.checked;
                    }
                    break;
                default:
                    // Nothing required
                    break;
            }
        }
        node.clicked = false;

        // Determine the lowest depth node that is clicked, this updates node.clicked
        // after the whole tree is traversed
        bool clicked = state.parent_clicked && Boxf(node.origin, node.origin+node.size).contains(mouse_pos);
        if (clicked && state.depth >= node_clicked_depth) {
            node_clicked = state.node;
            node_clicked_depth = state.depth;
        }

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
                    style.element.bg_color,
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
                        ? style.element.bg_color
                        :style.element.pressed_bg_color;
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
                geometry_renderer.queue_box(
                    normalized_depth,
                    Boxf(node.origin, node.origin+node.size),
                    style.element.bg_color,
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
            stack.emplace(child, clicked, state.depth + 1);
            child = tree[child].next;
        }
    }

    if (node_clicked != -1) {
        if (events.mouse_down) {
            node_pressed = node_clicked;
            node_focused = node_clicked;
            const auto& node = tree[node_clicked];
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
                    }
                    break;
                default:
                    break;
            }
        } else if (events.mouse_up) {
            if (node_clicked == node_pressed) {
                tree[node_clicked].clicked = true;
            }
            node_pressed = -1;
        }
    } else if (node_pressed != -1) {
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

    if (node_focused != -1) {
        const auto& node = tree[node_focused];
        switch (node.element) {
            case Element::TextInput:
                {
                    auto& element = elements.text_input[node.element_index];
                    if (events.key_down) {
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
                }
                break;
            default:
                break;
        }
    }

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    Vecf viewport_size(display_w, display_h);

    geometry_renderer.render(viewport_size);
    text_renderer.render(viewport_size);
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

} // namespace datagui
