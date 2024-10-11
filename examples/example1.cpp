#include <datagui/window.hpp>
#include <iostream>

int main() {
    datagui::Window window("Example 1");
    while (window.running()) {
        window.render_begin();
        window.vertical_layout("div1", 0, 0, {.padding = 10, .bg_color=datagui::Color::Gray(0.5)});
        window.text("text1", "Line 1", 0, {.bg_color=datagui::Color::Red()});
        window.text("text2", "Line 2");
        if (window.button("button1", "Click me!")) {
            std::cout << "Clicked!" << std::endl;
        }
        {
            window.horizontal_layout("checkbox_layout", 0, 0, {.padding = 10});
            if (window.checkbox("checkbox1")) {
                std::cout << "Checked!" << std::endl;
            }
            window.text("checkbox1_text", "Toggle me!");
            window.layout_end();
        }
        window.layout_end();
        window.render_end();
    }
    return 0;
}

