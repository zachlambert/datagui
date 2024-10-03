#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "datagui/font.hpp"


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
            vsync(0)
        {}
        static Config defaults() {
            Config config;
            config.font = Font::DejaVuSans;
            config.font_size = 18;
            config.width = 900;
            config.height = 600;
            config.vsync = true;
            return config;
        }
    };

    Window(const std::string& title, const Config& config = Config::defaults(), bool open_now=true):
        title(title),
        config(config),
        window(nullptr)
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
        return !glfwWindowShouldClose(window);
    }

    void open();
    void poll_events();
    void render_start();
    void render_end();
    void close();

private:
    const std::string title;
    const Config config;
    GLFWwindow* window;
    WindowInput input;
};

} // namespace datagui
