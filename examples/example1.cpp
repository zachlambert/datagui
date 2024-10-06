#include <datagui/window.hpp>

int main() {
    datagui::Window window("Example 1");
    while (window.running()) {
        window.poll_events();
        auto widget = window.render_start();
        widget.row(100, datagui::Color(1, 0.6, 0.7));
        widget.row(100, datagui::Color(0, 0.8, 1.0)).text("hello, world!", 200);
        window.render_end();
    }
    return 0;
}

