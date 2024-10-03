#include <datagui/window.hpp>

int main() {
    datagui::Window window("Example 1");
    while (window.running()) {
        window.poll_events();
        auto widget = window.render_start();
        widget.render();
        window.render_end();
    }
    return 0;
}

