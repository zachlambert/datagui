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

  Boxf box;
  box.lower = Vecf(50, 50);
  box.upper = Vecf(300, 300);

  while (window.running()) {
    window.render_begin();

    geometry.queue_box(box, Color::Red(), 5, Color::Black(), 0);
    text.queue_text(
        Vecf(400, 100),
        "Hello",
        Font::DejaVuSans,
        20,
        Color::Blue(),
        LengthWrap());
    text.queue_text(
        Vecf(400, 150),
        "Hello",
        Font::DejaVuSansMono,
        40,
        Color::Blue(),
        LengthWrap());
    text.queue_text(
        Vecf(400, 220),
        "Hello",
        Font::DejaVuSerif,
        60,
        Color::Blue(),
        LengthWrap());

    geometry.render(window.size());
    text.render(window.size());
    window.render_end();

    window.poll_events();
    for (const auto& event : window.mouse_events()) {
      switch (event.action) {
      case MouseAction::Press:
        std::cout << "Mouse press: " << int(event.button) << std::endl;
        break;
      case MouseAction::Release:
        std::cout << "Mouse release: " << int(event.button) << std::endl;
        break;
      default:
        break;
      }
    }
  }
}
