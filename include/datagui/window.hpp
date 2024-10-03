#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "datagui/font.hpp"
#include "datagui/geometry.hpp"
#include "datagui/renderer.hpp"


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


class Widget;

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
            vsync(false)
        {}
        static Config defaults() {
            Config config;
            config.font = Font::DejaVuSans;
            config.font_size = 18;
            config.width = 900;
            config.height = 600;
            config.vsync = false;
            return config;
        }
    };

    Window(const std::string& title, const Config& config = Config::defaults(), bool open_now = true):
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
        return window && !glfwWindowShouldClose(window);
    }

    void open();
    void poll_events();
    Widget render_start();
    void render_end();
    void close();

private:
    const std::string title;
    const Config config;
    GLFWwindow* window;
    Renderer renderer;
    WindowInput input;
};

class Widget {
public:
    Widget(Renderer& renderer, int depth, const Boxi& region):
        renderer(renderer),
        depth(depth),
        region(region),
        offset(Veci::Zero())
    {}

    void render() {
        // TEMP
        renderer.queue_box(depth, region, Color::Red());
    }

private:
    Renderer& renderer;
    int depth;
    Boxi region;
    Veci offset;
};

} // namespace datagui
