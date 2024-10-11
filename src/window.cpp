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

    std::cout << "New node: " << key << std::endl;

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
            std::cout << "Remove node: " << node.key << std::endl;
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
    stack.emplace(root_node, events.mouse_up, 0);

    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    Vecf mouse_pos(mx, my);

    int node_clicked = -1;
    int node_clicked_depth = -1;

    while (!stack.empty()) {
        State state = stack.top();
        const auto& node = nodes[state.node];
        bool clicked = state.parent_clicked && Boxf(node.origin, node.origin+node.size).contains(mouse_pos);

        if (clicked && state.depth >= node_clicked_depth) {
            node_clicked = state.node;
            node_clicked_depth = state.depth;
        }

        float normalized_depth = float(state.depth) / (max_depth + 1);

        stack.pop();

        switch (node.element) {
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
        std::cout << "Clicked: " << nodes[node_clicked].key << std::endl;
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

} // namespace datagui
