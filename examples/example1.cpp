#include <datagui/window.hpp>

int main() {
    datagui::Window window("Example 1");
    while (window.running()) {
        window.poll_events();
        window.render_start();
        window.render_end();
    }
    return 0;
}

