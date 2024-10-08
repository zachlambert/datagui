#include <datagui/window.hpp>
#include <iostream>

int main() {
    datagui::Window window("Example 1");
    while (window.running()) {
        window.poll_events();
        window.render_begin();
        window.vertical_layout(0, 0, {.padding = 10, .bg_color=datagui::Color::Gray(0.5)});
        {
            window.text("Line 1", 0, {.bg_color=datagui::Color::Red()});
            window.text("Line 2");
        }
        window.layout_end();
        window.render_end();
    }
    return 0;
}

