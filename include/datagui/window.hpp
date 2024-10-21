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
#include "datagui/internal/selection.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/internal/renderers.hpp"

#include "datagui/element/button.hpp"
#include "datagui/element/checkbox.hpp"
#include "datagui/element/linear_layout.hpp"
#include "datagui/element/text.hpp"
#include "datagui/element/text_input.hpp"


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

    bool vertical_layout(
        const std::string& key,
        float width=0,
        float height=0);

    bool horizontal_layout(
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

    const bool* checkbox(
        const std::string& key);

    const std::string* text_input(
        const std::string& key,
        const std::string& default_text = "",
        float max_width = -1);

    void hidden(const std::string& key);

private:
    void open();
    void close();

    void delete_element(Element element, int index);
    ElementSystem& get_elements(const Node& node);
    void event_handling();

    const Config config;
    const Style style;
    GLFWwindow* window;
    Vecf window_size;

    FontStructure font;
    TextSelection selection;
    Renderers renderers;

    Tree tree;
    ButtonSystem buttons;
    CheckboxSystem checkboxes;
    LinearLayoutSystem linear_layouts;
    TextSystem texts;
    TextInputSystem text_inputs;
};

} // namespace datagui
