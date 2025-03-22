#include <datagui/visual/font.hpp>
#include <datagui/visual/geometry_renderer.hpp>
#include <datagui/visual/text_renderer.hpp>
#include <datagui/visual/window.hpp>
#include <iostream>

int main() {
  using namespace datagui;

  Window window;
  FontManager font_manager;
  GeometryRenderer geometry;
  TextRenderer text(font_manager);
  geometry.init();
  text.init();
  font_manager.font_structure(Font::DejaVuSans, 20);

  Boxf box;
  box.lower = Vecf(50, 50);
  box.upper = Vecf(300, 300);

  while (window.running()) {
    window.render_begin();

    geometry.queue_box(box, Color::Red(), 5, Color::Black());
    text.queue_text("Hello", Vecf(400, 50), Font::DejaVuSans, 20, Color::Blue(), 500);

    geometry.render(window.size());
    text.render(window.size());
    window.render_end();

    window.poll_events();
    if (window.events().mouse.press) {
      std::cout << "Mouse down" << std::endl;
    }
    if (window.events().mouse.release) {
      std::cout << "Mouse release" << std::endl;
    }
  }
}
