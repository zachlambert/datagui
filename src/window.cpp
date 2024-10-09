#include "datagui/window.hpp"

#include <unordered_map>
#include <stack>
#include <iostream>


namespace datagui {

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
    float width,
    float height,
    const element::VerticalLayout::Props& props)
{
    int node = create_node(Element::VerticalLayout, active_node);
    if (nodes[node].element_index == -1) {
        nodes[node].element_index = elements.vertical_layout.size();
        elements.vertical_layout.emplace_back(Vecf(width, height), props);
    } else {
        auto& element = elements.vertical_layout[nodes[node].element_index];
        element.input_size = Vecf(width, height);
        element.props = props;
    }

    active_node = node;
    depth++;
    max_depth = std::max(max_depth, depth);
}

void Window::horizontal_layout(
    float width,
    float height,
    const element::HorizontalLayout::Props& props)
{
    int node = create_node(Element::HorizontalLayout, active_node);
    if (nodes[node].element_index == -1) {
        nodes[node].element_index = elements.horizontal_layout.size();
        elements.horizontal_layout.emplace_back(Vecf(width, height), props);
    } else {
        auto& element = elements.horizontal_layout[nodes[node].element_index];
        element.input_size = Vecf(width, height);
        element.props = props;
    }

    active_node = node;
    depth++;
    max_depth = std::max(max_depth, depth);
}

void Window::layout_end() {
    if (active_node == -1) {
        throw std::runtime_error("Called end too many times");
    }
    active_node = nodes[active_node].parent;
    depth--;
}

void Window::text(
    const std::string& text,
    float max_width,
    const element::Text::Props& props)
{
    int node = create_node(Element::Text, active_node);
    if (nodes[node].element_index == -1) {
        nodes[node].element_index = elements.text.size();
        elements.text.emplace_back(text, max_width, props);
    } else {
        auto& element = elements.text[nodes[node].element_index];
        element.text = text;
        element.max_width = max_width;
        element.props = props;
    }
}

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

void Window::render_begin() {
    glClearColor(0.5, 0.5, 0.5, 1);
    glClearDepth(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    vertical_layout(display_w, display_h);
}

void Window::render_end() {
    layout_end();
    if (active_node != -1) {
        throw std::runtime_error("Didn't call layout_... and layout_end the same number of times");
    }

    calculate_sizes_up();
    calculate_sizes_down();
    render_tree();

    glfwSwapBuffers(window);

    prev_nodes = std::move(nodes);
    // elements.vertical_layout.clear();
    // elements.horizontal_layout.clear();
    // elements.text.clear();
    max_depth = 0;
    depth = 0;
}

int Window::create_node(Element element, int parent) {
    int node_index = nodes.size();
    if (parent != -1) {
        if (nodes[parent].first_child == -1) {
            nodes[parent].first_child = node_index;
        } else {
            nodes[nodes[parent].last_child].next = node_index;
        }
        nodes[parent].last_child = node_index;
    }

    nodes.emplace_back(element, parent);
    return node_index;
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
            case Element::Text:
                throw std::runtime_error("Text element shouldn't have children");
                break;
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
            }

            stack.push(child);
            child = child_node.next;
        }
    }
}

void Window::render_tree() {
    std::stack<int> stack;
    stack.push(0);

    auto get_depth = [&]() -> float {
        return float(stack.size() + 1) / (max_depth + 3);
    };

    while (!stack.empty()) {
        const auto& node = nodes[stack.top()];
        stack.pop();

        switch (node.element) {
        case Element::Text:
            {
                const auto& element = elements.text[node.element_index];
                geometry_renderer.queue_box(
                    get_depth(),
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
                    get_depth(),
                    element.props.text_color
                );
            }
            break;
        case Element::VerticalLayout:
            {
                const auto& element = elements.vertical_layout[node.element_index];
                geometry_renderer.queue_box(
                    get_depth(),
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
                    get_depth(),
                    Boxf(node.origin, node.origin+node.size),
                    element.props.bg_color,
                    0,
                    Color::Black(),
                    0
                );
            }
            break;
        }

        if (node.first_child == -1) {
            continue;
        }
        int child = node.first_child;
        while (child != -1) {
            stack.push(child);
            child = nodes[child].next;
        }
    }

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    Vecf viewport_size(display_w, display_h);

    geometry_renderer.render(viewport_size);
    text_renderer.render(viewport_size);
}

} // namespace datagui
