#include <datagui/window.hpp>
#include <iostream>

int main() {
    datagui::Window window("Example 1");
    int i = 0;
    while (window.running()) {
        window.poll_events();
        window.render_begin();
        if (i < 60) {
            window.vertical_layout("div1", 0, 0, {.padding = 10, .bg_color=datagui::Color::Gray(0.5)});
            window.text("text1", "Line 1", 0, {.bg_color=datagui::Color::Red()});
            window.text("text2", "Line 2");
            window.layout_end();
        } else {
            window.vertical_layout("div2", 0, 0, {.padding = 10, .bg_color=datagui::Color::Gray(0.2)});
            window.text("text1", "Line 1", 0, {.bg_color=datagui::Color::Blue()});
            window.text("text2", "Line 2");
            window.layout_end();
        }
        window.render_end();
        i++;
    }
    return 0;
}

