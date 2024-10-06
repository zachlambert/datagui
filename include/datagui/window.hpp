#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "datagui/font.hpp"
#include "datagui/geometry.hpp"
#include "datagui/geometry_renderer.hpp"
#include "datagui/text_renderer.hpp"


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

struct Renderers {
    GeometryRenderer geometry;
    TextRenderer text;
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
            config.font_size = 28;
            config.width = 900;
            config.height = 600;
            config.vsync = true;
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
    Renderers renderers;
    WindowInput input;
};

class Widget {
public:
    Widget(Renderers& renderers, int depth, const Boxf& outer_region, const Color& bg_color):
        renderers(renderers),
        depth(depth),
        region(outer_region),
        offset(Vecf::Zero()),
        border_size(10),
        padding_size(10)
    {
        renderers.geometry.queue_box(depth, region, bg_color, border_size, Color::Gray(0.25), 0);
        region.lower += Vecf::Constant(border_size + padding_size);
        region.upper -= Vecf::Constant(border_size + padding_size);
    }

    Widget row(float height, const Color& bg_color = Color::Gray(0.75)) {
        Vecf size(region.upper.x-region.lower.x, height);
        Widget widget(
            renderers,
            depth+1,
            Boxf(region.lower+offset, region.lower+offset+size),
            bg_color
        );
        offset.y += height;
        return widget;
    }

    void text(const std::string& text, float line_width) {
        renderers.text.queue_text(depth+1, region.lower, text, line_width);
    }

private:
    Renderers& renderers;
    int depth;
    Boxf region;
    Vecf offset;
    float border_size;
    float padding_size;
};

} // namespace datagui
