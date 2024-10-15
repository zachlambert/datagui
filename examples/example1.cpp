#include <datagui/window.hpp>
#include <iostream>

int main() {
    datagui::Window::Config config;
    config.title = "Datagui Example 1";
    datagui::Style style;

    datagui::Window window(config, style);
    window.open();

    while (window.running()) {
        window.render_begin();
        window.vertical_layout("div1");
        window.text("text1", "Line 1");
        window.text("text2", "Line 2");
        if (window.button("button1", "Click me!")) {
            std::cout << "Clicked!" << std::endl;
        }
        {
            window.horizontal_layout("checkbox_layout");
            if (window.checkbox("checkbox1")) {
                std::cout << "Checked!" << std::endl;
            }
            window.text("checkbox1_text", "Toggle me!");
            window.layout_end();
        }
        window.text_input("input1", "hello");
        window.text_input("input2", "a multi-line input?", 200);
        window.layout_end();
        window.render_end();
    }
    return 0;
}

