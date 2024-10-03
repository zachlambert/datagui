#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "datagui/font.hpp"
#include "datagui/widget.hpp"


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
};

class Window {
public:
    Window(const std::string& title):
        initialised(false),
        window(nullptr),
        title(title)
    {}
    ~Window()
    {
        shutdown();
    }

    void add_widget(const std::string& name, std::shared_ptr<Widget> widget)
    {
        this->widgets.emplace(name, widget);
    }

    bool running() const
    {
        return !glfwWindowShouldClose(window);
    }

    void run_once()
    {
        poll_events();
        render_start();
        for (auto& [name, widget]: widgets) {
            ImGui::Begin(name.c_str());
            widget->render();
            ImGui::End();
        }
        render_end();
    }

    bool run()
    {
        if (!init()) {
            return false;
        }
        while (running()) {
            run_once();
        }
        shutdown();
        return true;
    }


    bool init();
    void poll_events();
    void render_start();
    void render_end();
    void shutdown();

private:
    struct Config {
        Font font = Font::DejaVuSans;
        int font_size = 18;
        int width = 900;
        int height = 600;
        bool vsync = true;
    };

    bool initialised;
    GLFWwindow* window;
    std::string title;
    Config config;
    WindowInput input;

    std::unordered_map<std::string, std::shared_ptr<Widget>> widgets;
};

} // namespace datagui
