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
    void event_handling();
    void render_tree();

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
