#include <datagui/window.hpp>
#include <iostream>

int main() {
    datagui::Window::Config config;
    config.title = "Datagui Example 1";
    datagui::Style style;

    datagui::Window window(config, style);

    while (window.running()) {
        window.render_begin();
#if 0
        window.vertical_layout("div1");
        window.text("text1", "Line 1");
        window.text("text2", "Line 2");
        if (window.button("button1", "Click me!")) {
            std::cout << "Clicked!" << std::endl;
        }
        {
            window.horizontal_layout("checkbox_layout");
            if (auto value = window.checkbox("checkbox1")) {
                std::cout << "Toggle -> " << (*value ? "on" : "off") << std::endl;
            }
            window.text("checkbox1_text", "Toggle me!");
            window.layout_end();
        }
        if (auto value = window.text_input("input1", "min input", 0)) {
            std::cout << "Input 1 changed -> " << *value << std::endl;
        }
        window.text_input("input2", "expand input", -1);
        window.text_input("input3", "finite input", 200);
        window.layout_end();
#endif
        window.render_end();
    }
    return 0;
}

