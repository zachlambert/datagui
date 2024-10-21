#include <datagui/window.hpp>
#include <iostream>

int main() {
    datagui::Window::Config config;
    config.title = "Datagui Example 1";
    datagui::Style style;

    datagui::Window window(config, style);
    bool checkbox1 = false;

    while (window.running()) {
        window.render_begin();
        if (window.vertical_layout("div1")) {
            window.text("text1", "Line 1");
            window.text("text2", "Line 2");
            if (window.button("button1", "Click me!")) {
                std::cout << "Clicked!" << std::endl;
            }
            if (window.horizontal_layout("checkbox_layout")) {
                if (auto value = window.checkbox("checkbox1")) {
                    checkbox1 = *value;
                    std::cout << "Toggle -> " << (*value ? "on" : "off") << std::endl;
                }
                window.text("checkbox1_text", "Toggle me!");
                window.layout_end();
            }
            if (checkbox1) {
                window.text("Checkbox text", "Hello");
            } else {
                window.hidden("Checkbox text");
            }
            if (auto value = window.text_input("input1", "min input", 0)) {
                std::cout << "Input 1 changed -> " << *value << std::endl;
            }
            if (auto value = window.text_input("input2", "expand input", -1)) {
                std::cout << "Input 2 changed -> " << *value << std::endl;
            }
            if (auto value = window.text_input("input3", "finite input", 200)) {
                std::cout << "Input 3 changed -> " << *value << std::endl;
            }
            window.layout_end();
        }
        window.render_end();
    }
    return 0;
}

