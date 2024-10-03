#include "datagui/window.hpp"

#include <iostream>
#include <unordered_map>
#include <filesystem>


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

    renderer.init();
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

Widget Window::render_start() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    return Widget(
        renderer,
        0,
        Boxf(
            Vecf::Zero(),
            Vecf(display_w, display_h)
        ),
        Color::Gray(0.8)
    );
}

void Window::render_end() {
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    renderer.render(Vecf(display_w, display_h));
    glfwSwapBuffers(window);
}

} // namespace datagui
