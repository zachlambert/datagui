#include "datagui/window.hpp"

#include <unordered_map>
#include <stack>
#include <iostream>


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

void Window::open() {
    if (!glfwInit()) {
        throw Error("Failed to initialize glfw");
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    window = glfwCreateWindow(config.width, config.height, title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        throw Error("Failed to create window");
    }

    glfwMakeContextCurrent(window);

    if (config.vsync) {
        glfwSwapInterval(1);
    } else {
        glfwSwapInterval(0);
    }

    if (glewInit() != GLEW_OK) {
        throw Error("Failed to initialise glew");
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);

    geometry_renderer.init();
    text_renderer.init(config.font, config.font_size);

    active_windows.emplace_back(window, this);
    glfwSetMouseButtonCallback(window, Window::glfw_mouse_button_callback);
    glfwSetKeyCallback(window, Window::glfw_key_callback);
}

void Window::close() {
    if (!window) {
        throw Error("Window is already closed");
    }

    glfwMakeContextCurrent(window);
    glfwDestroyWindow(window);
    glfwTerminate();
    window = nullptr;
}

void Window::vertical_layout(
    const std::string& key,
    float width,
    float height,
    const element::VerticalLayout::Props& props)
{
    int node = visit_node(key, Element::VerticalLayout, true);
    if (nodes[node].element_index == -1) {
        nodes[node].element_index = elements.vertical_layout.emplace(Vecf(width, height), props);
    }
}

void Window::horizontal_layout(
    const std::string& key,
    float width,
    float height,
    const element::HorizontalLayout::Props& props)
{
    int node = visit_node(key, Element::HorizontalLayout, true);
    if (nodes[node].element_index == -1) {
        nodes[node].element_index = elements.horizontal_layout.emplace(Vecf(width, height), props);
    }
}

void Window::layout_end() {
    if (active_nodes.empty()) {
        throw std::runtime_error("Called end too many times");
    }

    // Remove nodes that weren't visited this iteration

    int iter = nodes[active_nodes.top()].first_child;
    while (iter != -1) {
        int next = nodes[iter].next;
        if (nodes[iter].iteration != iteration) {
            remove_node(iter);
        }
        iter = next;
    }

    active_nodes.pop();
}

void Window::text(
    const std::string& key,
    const std::string& text,
    float max_width,
    const element::Text::Props& props)
{
    int node = visit_node(key, Element::Text, false);
    if (nodes[node].element_index == -1) {
        nodes[node].element_index = elements.text.emplace(text, max_width, props);
    }
}

bool Window::button(
    const std::string& key,
    const std::string& text,
    float max_width,
    const element::Button::Props& props)
{
    int node = visit_node(key, Element::Button, false);
    if (nodes[node].element_index == -1) {
        nodes[node].element_index = elements.button.emplace(text, max_width, props);
    }
    return nodes[node].clicked;
}

bool Window::checkbox(
    const std::string& key,
    const element::Checkbox::Props& props)
{
    auto& node = nodes[visit_node(key, Element::Checkbox, false)];
    if (node.element_index == -1) {
        node.element_index = elements.checkbox.emplace(props);
    }
    const auto& element = elements.checkbox[node.element_index];
    return element.checked;
}

bool Window::text_input(
    const std::string& key,
    const std::string& default_text,
    float max_width,
    int num_lines,
    const element::TextInput::Props& props)
{
    auto& node = nodes[visit_node(key, Element::TextInput, false)];
    if (node.element_index == -1) {
        node.element_index = elements.text_input.emplace(
            default_text,
            max_width,
            num_lines,
            props
        );
    }
    const auto& element = elements.text_input[node.element_index];
    return element.changed;
}

#if 0
void Window::poll_events() {
    glfwPollEvents();

    GLFWgamepadstate gamepad_state;
    input.valid = glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state);
    if (input.valid) {
        auto threshold = [](double axis) -> double {
            const double min = 0.2;
            if (axis < -min) return (axis + min) / (1.0 - min);
            if (axis > min) return (axis - min) / (1.0 - min);
            return 0;
        };
        input.axes[(size_t)WindowInput::Axis::LEFT_X] = -threshold(gamepad_state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]);
        input.axes[(size_t)WindowInput::Axis::LEFT_Y] = -threshold(gamepad_state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
        input.axes[(size_t)WindowInput::Axis::RIGHT_X] = -threshold(gamepad_state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]);
        input.axes[(size_t)WindowInput::Axis::RIGHT_Y] = -threshold(gamepad_state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);

        input.buttons[(size_t)WindowInput::Button::A] = gamepad_state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS;
        input.buttons[(size_t)WindowInput::Button::B] = gamepad_state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS;
        input.buttons[(size_t)WindowInput::Button::X] = gamepad_state.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS;
        input.buttons[(size_t)WindowInput::Button::Y] = gamepad_state.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS;
        input.buttons[(size_t)WindowInput::Button::LB] = gamepad_state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS;
        input.buttons[(size_t)WindowInput::Button::RB] = gamepad_state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS;
    }
}
#endif

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
    if (!active_nodes.empty()) {
        throw std::runtime_error("Didn't call layout_... and layout_end the same number of times");
    }

    glfwPollEvents();

    calculate_sizes_up();
    calculate_sizes_down();
    render_tree();

    glfwSwapBuffers(window);

    events.clear();
    max_depth = 0;
    iteration++;
}

int Window::visit_node(const std::string& key, Element element, bool enter) {
    int parent = active_nodes.empty() ? -1 : active_nodes.top();

    if (parent != -1) {
        int iter = nodes[parent].first_child;
        while (iter != -1) {
            if (nodes[iter].key == key) {
                if (nodes[iter].iteration == iteration) {
                    throw std::runtime_error("Visited the same node twice during the same iteration");
                }
                nodes[iter].reset(iteration);
                if (enter) {
                    active_nodes.push(iter);
                    max_depth = std::max<int>(max_depth, active_nodes.size());
                }
                return iter;
            }
            iter = nodes[iter].next;
        }
    } else if (root_node != -1) {
        if (enter) {
            active_nodes.push(root_node);
            max_depth = std::max<int>(max_depth, active_nodes.size());
        }
        return root_node;
    }

    int node = nodes.emplace(key, element, parent, iteration);
    if (nodes.size() == 1) {
        root_node = node;
    }

    if (parent != -1) {
        nodes[node].prev = nodes[parent].last_child;
        if (nodes[parent].first_child == -1) {
            nodes[parent].first_child = node;
        } else {
            nodes[nodes[parent].last_child].next = node;
        }
        nodes[parent].last_child = node;
    }

    if (enter) {
        active_nodes.push(node);
        max_depth = std::max<int>(max_depth, active_nodes.size());
    }

    return node;
}

void Window::remove_node(int root_node) {
    std::stack<int> stack;
    stack.push(root_node);
    while (!stack.empty()) {
        int node_index = stack.top();
        const auto& node = nodes[node_index];
        if (node.first_child == -1) {
            stack.pop();
            if (node.prev != -1) {
                nodes[node.prev].next = node.next;
            } else if (node.parent != -1) {
                nodes[node.parent].first_child = node.next;
            }
            if (node.next != -1) {
                nodes[node.next].prev = node.prev;
            } else if (node.parent != -1) {
                nodes[node.parent].last_child = node.prev;
            }
            switch (node.element) {
            case Element::VerticalLayout:
                elements.vertical_layout.pop(node.element_index);
                break;
            case Element::HorizontalLayout:
                elements.horizontal_layout.pop(node.element_index);
                break;
            case Element::Text:
                elements.text.pop(node.element_index);
                break;
            case Element::Button:
                elements.button.pop(node.element_index);
                break;
            case Element::Checkbox:
                elements.checkbox.pop(node.element_index);
                break;
            case Element::TextInput:
                elements.text_input.pop(node.element_index);
                break;
            }
            nodes.pop(node_index);
            continue;
        }
        int iter = node.first_child;
        while (iter != -1) {
            stack.push(iter);
            iter = nodes[iter].next;
        }
    }
}

void Window::calculate_sizes_up() {
    /*
      - Traverse down the tree, where if non-leaf node is reached, all the
         child nodes are processed first.
      - Each node must calculate it's 'fixed_size' and 'dynamic_size', where:
        - For leaf nodes, these are defined by the element and it's properties
        - For branch nodes, these are defined by the element type, it's properties
          and the fixed_size/dynamic_size of the children
    */

    struct State {
        int index;
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
        Node& node = nodes[state.index];

        // If the node has children, process these first
        if (node.first_child != -1 && state.first_visit) {
            state.first_visit = false;
            int child = node.first_child;
            while (child != -1) {
                stack.emplace(child);
                child = nodes[child].next;
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
                        node.fixed_size.x = std::max(node.fixed_size.x, nodes[child].fixed_size.x);
                        node.dynamic_size.x = std::max(node.dynamic_size.x, nodes[child].dynamic_size.x);
                        child = nodes[child].next;
                    }
                    node.fixed_size.x += 2 * element.props.padding;

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
                        node.fixed_size.y += nodes[child].fixed_size.y;
                        node.dynamic_size.y += nodes[child].dynamic_size.y;
                        child = nodes[child].next;
                    }
                    node.fixed_size.y += 2 * element.props.padding;
                    node.fixed_size.y += (count - 1) * element.props.padding;

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
                        node.fixed_size.x += nodes[child].fixed_size.x;
                        node.dynamic_size.x += nodes[child].dynamic_size.x;
                        child = nodes[child].next;
                    }
                    node.fixed_size.x += 2 * element.props.padding;
                    node.fixed_size.x += (count - 1) * element.props.padding;

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
                        node.fixed_size.y = std::max(node.fixed_size.y, nodes[child].fixed_size.y);
                        node.dynamic_size.y = std::max(node.dynamic_size.y, nodes[child].dynamic_size.y);
                        child = nodes[child].next;
                    }
                    node.fixed_size.y += 2 * element.props.padding;
                    node.fixed_size.y += (count - 1) * element.props.padding;

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
                    element.props.line_height_factor);
            }
            break;
        case Element::Button:
            {
                const auto& element = elements.button[node.element_index];

                node.fixed_size = text_renderer.text_size(
                    element.text,
                    element.max_width,
                    element.props.line_height_factor);
                node.fixed_size += Vecf::Constant((element.props.border_width + element.props.padding) * 2);
            }
            break;
        case Element::Checkbox:
            {
                const auto& element = elements.checkbox[node.element_index];
                node.fixed_size = Vecf::Constant(text_renderer.get_font_size() * element.props.size_factor);
            }
            break;
        case Element::TextInput:
            {
                const auto& element = elements.text_input[node.element_index];

                node.fixed_size = text_renderer.text_size(
                    element.text,
                    element.max_width,
                    element.props.line_height_factor);
                node.fixed_size += Vecf::Constant(
                    2 * (element.props.border_width + element.props.padding));
            }
            break;
        };
    }
}

void Window::calculate_sizes_down() {
    std::stack<int> stack;
    stack.push(0);

    nodes[0].size = nodes[0].fixed_size;
    nodes[0].origin = Vecf::Zero();

    while (!stack.empty()) {
        const auto& parent = nodes[stack.top()];
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
                    offset.x += element.props.padding;
                    offset.y += element.props.padding;
                }
                break;
            case Element::HorizontalLayout:
                {
                    const auto& element = elements.horizontal_layout[parent.element_index];
                    offset.x += element.props.padding;
                    offset.y += element.props.padding;
                }
                break;
            default:
                break;
        }

        int child = parent.first_child;
        while (child != -1) {
            auto& child_node = nodes[child];
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
                    offset.y += child_node.size.y + element.props.padding;
                }
                break;
            case Element::HorizontalLayout:
                {
                    const auto& element = elements.horizontal_layout[parent.element_index];
                    offset.x += child_node.size.x + element.props.padding;
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
    stack.emplace(root_node, events.mouse_up || events.mouse_down, 0);

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    Vecf mouse_pos(mx, my);

    int node_clicked = -1;
    int node_clicked_depth = -1;

    while (!stack.empty()) {
        State state = stack.top();
        auto& node = nodes[state.node];

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

        float normalized_depth = float(state.depth) / (max_depth + 1);

        stack.pop();

        switch (node.element) {
        case Element::VerticalLayout:
            {
                const auto& element = elements.vertical_layout[node.element_index];
                geometry_renderer.queue_box(
                    normalized_depth,
                    Boxf(node.origin, node.origin+node.size),
                    element.props.bg_color,
                    0,
                    Color::Black(),
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
                    element.props.bg_color,
                    0,
                    Color::Black(),
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
                    element.props.bg_color,
                    0,
                    Color::Black(),
                    0
                );
                text_renderer.queue_text(
                    element.text,
                    element.max_width,
                    element.props.line_height_factor,
                    node.origin,
                    normalized_depth,
                    element.props.text_color
                );
            }
            break;
        case Element::Button:
            {
                const auto& element = elements.button[node.element_index];
                Color bg_color = element.props.bg_color;
                if (node_pressed == state.node) {
                    bg_color.r *= 0.5; // TODO: Class member variable
                    bg_color.g *= 0.5;
                    bg_color.b *= 0.5;
                }
                geometry_renderer.queue_box(
                    normalized_depth,
                    Boxf(node.origin, node.origin+node.size),
                    bg_color,
                    element.props.border_width,
                    element.props.border_color
                );
                text_renderer.queue_text(
                    element.text,
                    element.max_width,
                    element.props.line_height_factor,
                    node.origin + Vecf::Constant(element.props.border_width + element.props.padding),
                    normalized_depth,
                    element.props.text_color
                );
            }
            break;
        case Element::Checkbox:
            {
                const auto& element = elements.checkbox[node.element_index];
                geometry_renderer.queue_box(
                    normalized_depth,
                    Boxf(node.origin, node.origin+node.size),
                    element.props.bg_color,
                    element.props.border_width,
                    element.props.border_color,
                    0
                );
                if (element.checked) {
                    float offset = element.props.border_width + 2;
                    geometry_renderer.queue_box(
                        normalized_depth,
                        Boxf(
                            node.origin + Vecf::Constant(offset),
                            node.origin + node.size - Vecf::Constant(offset)
                        ),
                        element.props.icon_color,
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
                    element.props.bg_color,
                    element.props.border_width,
                    state.node == node_focused
                        ? element.props.focus_color
                        : element.props.border_color,
                    0
                );
                Vecf text_origin =
                    node.origin
                    + Vecf::Constant(
                        element.props.border_width + element.props.padding);

                if (state.node == node_focused) {
                    float line_height = text_renderer.get_font_size() * element.props.line_height_factor;

                    if (element.cursor_begin == element.cursor_end) {
                        // TODO: Blink
                        geometry_renderer.queue_box(
                            normalized_depth,
                            Boxf(element.cursor_begin.second-Vecf(float(element.props.cursor_width)/2, 0), element.cursor_begin.second + Vecf(element.props.cursor_width, line_height)),
                            element.props.cursor_color,
                            0,
                            Color::Black(),
                            0
                        );
                    } else {
                        Vecf from, to;
                        if (element.cursor_begin.second.y < element.cursor_end.second.y || element.cursor_begin.second.y == element.cursor_end.second.y && element.cursor_begin.second.x <= element.cursor_end.second.x) {
                            from = element.cursor_begin.second;
                            to = element.cursor_end.second;
                        } else {
                            from = element.cursor_end.second;
                            to = element.cursor_begin.second;
                        }

                        while (true) {
                            Vecf line_to;
                            if (from.y == to.y) {
                                line_to = to;
                            } else {
                                line_to.x = text_origin.x + node.size.x - 2*(element.props.border_width + element.props.padding);
                                line_to.y = from.y;
                            }

                            geometry_renderer.queue_box(
                                normalized_depth,
                                Boxf(from, line_to + Vecf(0, line_height)),
                                element.props.highlight_color,
                                0,
                                Color::Black(),
                                0
                            );

                            if (from.y >= to.y) {
                                break;
                            }
                            from.y += line_height;
                            from.x = text_origin.x;
                        }
                    }

                }
                text_renderer.queue_text(
                    element.text,
                    element.max_width,
                    element.props.line_height_factor,
                    text_origin,
                    normalized_depth,
                    element.props.text_color
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
            child = nodes[child].next;
        }
    }

    if (node_clicked != -1) {
        if (events.mouse_down) {
            node_pressed = node_clicked;
            node_focused = node_clicked;
            const auto& node = nodes[node_clicked];
            switch (node.element) {
                case Element::TextInput:
                    {
                        auto& element = elements.text_input[node.element_index];
                        element.cursor_begin = text_renderer.cursor_offset(
                            node.origin + Vecf::Constant(
                                element.props.border_width + element.props.padding),
                            element.text,
                            node.size.x - (element.props.border_width + element.props.padding),
                            element.props.line_height_factor,
                            mouse_pos
                        );
                        element.cursor_end = element.cursor_begin;
                    }
                    break;
                default:
                    break;
            }
        } else if (events.mouse_up) {
            if (node_clicked == node_pressed) {
                nodes[node_clicked].clicked = true;
            }
            node_pressed = -1;
        }
    } else if (node_pressed != -1) {
        const auto& node = nodes[node_pressed];
        switch (node.element) {
            case Element::TextInput:
                {
                    auto& element = elements.text_input[node.element_index];
                    element.cursor_end = text_renderer.cursor_offset(
                        node.origin + Vecf::Constant(
                            element.props.border_width + element.props.padding),
                        element.text,
                        node.size.x - (element.props.border_width + element.props.padding),
                        element.props.line_height_factor,
                        mouse_pos
                    );
                }
                break;
            default:
                break;
        }
    }

    if (node_focused != -1) {
        const auto& node = nodes[node_focused];
        switch (node.element) {
            case Element::TextInput:
                {
                    auto& element = elements.text_input[node.element_index];
                    if (events.key_down) {
                        std::cout << "---\n";
                        if (events.key == GLFW_KEY_BACKSPACE) {
                            int from = element.cursor_begin.first;
                            int to = element.cursor_end.first;
                            if (from > to) {
                                std::swap(from, to);
                            }
                            if (from != -1 && to != -1) {
                                if (from != to) {
                                    std::cout << from << "->" << to << std::endl;
                                    element.text.erase(element.text.begin() + from, element.text.begin() + to);
                                    // TODO: Update cursor
                                } else if (from > 0) {
                                    std::cout << (from-1) << std::endl;
                                    element.text.erase(element.text.begin() + (from-1));
                                    // TODO: Update cursor
                                }
                            }
                        }
                        std::cout << "key: " << events.key << std::endl;
                        std::cout << "mods: " << events.mods << std::endl;
                        std::cout << "char: " << char(events.key) << std::endl;
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
