#pragma once

#include <array>
#include <memory>
#include <stack>
#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <variant>

#include "datagui/style.hpp"
#include "datagui/geometry.hpp"
#include "datagui/internal/vector_map.hpp"
#include "datagui/internal/geometry_renderer.hpp"
#include "datagui/internal/text_renderer.hpp"

#include "datagui/element.hpp"
#include "datagui/element/button.hpp"
#include "datagui/element/checkbox.hpp"
#include "datagui/element/horizontal_layout.hpp"
#include "datagui/element/text.hpp"
#include "datagui/element/text_input.hpp"
#include "datagui/element/vertical_layout.hpp"


namespace datagui {

class Window {
public:
    struct Config {
        std::string title;
        int width;
        int height;
        bool vsync;
        bool resizable;
        Config():
            title("datagui"),
            width(900),
            height(600),
            vsync(false),
            resizable(true)
        {}
    };

    Window(const Config& config = Config(), const Style& style = Style());
    ~Window();

    bool running() const;
    void render_begin();
    void render_end();

    void vertical_layout(
        const std::string& key,
        float width=0,
        float height=0);

    void horizontal_layout(
        const std::string& key,
        float width=0,
        float height=0);

    void layout_end();

    void text(
        const std::string& key,
        const std::string& text,
        float max_width = 0);

    bool button(
        const std::string& key,
        const std::string& text,
        float max_width = 0);

    bool checkbox(
        const std::string& key);

    bool text_input(
        const std::string& key,
        const std::string& default_text = "",
        float max_width = -1);

private:
    void open();
    void close();

    void delete_element(Element element, int index);

    void mouse_button_callback(int button, int action, int mods);
    void key_callback(int key, int scancode, int action, int mods);
    void char_callback(unsigned int codepoint);
    void event_handling();

    static std::vector<std::pair<GLFWwindow*, Window*>> active_windows;
    static void glfw_mouse_button_callback(GLFWwindow* callback_window, int button, int action, int mods);
    static void glfw_key_callback(GLFWwindow* callback_window, int key, int scancode, int action, int mods);
    static void glfw_char_callback(GLFWwindow* callback_window, unsigned int codepoint);

    void render_tree();

    struct Events {
        bool mouse_down;
        bool mouse_up;
        bool key_down;
        bool key_up;
        int key;
        int mods;
        bool has_char;
        char char_value;
        Events():
            mouse_down(false),
            mouse_up(false),
            key_down(false),
            key_up(false),
            key(-1),
            mods(0),
            has_char(false),
            char_value(0)
        {}
        void clear() {
            mouse_down = false;
            mouse_up = false;
            key_down = false;
            key_up = false;
            key = -1;
            mods = 0;
            has_char = false;
            char_value = 0;
        }
    };
    Events events;

    struct {
        VectorMap<VerticalLayout> vertical_layout;
        VectorMap<HorizontalLayout> horizontal_layout;
        VectorMap<Text> text;
        VectorMap<Button> button;
        VectorMap<Checkbox> checkbox;
        VectorMap<TextInput> text_input;
    } elements;

    const Config config;
    const Style style;
    GLFWwindow* window;

    GeometryRenderer geometry_renderer;
    TextRenderer text_renderer;

    Tree tree;
    Vecf window_size;

    TextStructure cursor_text;
    CursorPos cursor_begin;
    CursorPos cursor_end;
};

} // namespace datagui
