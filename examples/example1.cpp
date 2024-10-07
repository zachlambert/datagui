#include <datagui/window.hpp>
#include <iostream>

int main() {
    datagui::Window window("Example 1");
    while (window.running()) {
        window.poll_events();
        window.render_begin();
        // TODO: Breaks with another layout inside
        // window.linear_layout(false).padding(10);
        // {
        window.text("Line 1");
        window.text("Line 2");
        // }
        // window.layout_end();
        window.render_end();
    }
    return 0;
}

