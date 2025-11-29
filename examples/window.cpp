#include <datagui/visual/font_manager.hpp>
#include <datagui/visual/geometry_renderer.hpp>
#include <datagui/visual/text_renderer.hpp>
#include <datagui/visual/window.hpp>
#include <iostream>

int main() {
  using namespace datagui;

  Window window;
  auto font_manager = std::make_shared<FontManager>();
  GeometryRenderer geometry;
  TextRenderer text;
  geometry.init();
  text.init(font_manager);

  Box2 box;
  box.lower = Vec2(50, 50);
  box.upper = Vec2(300, 300);

  while (window.running()) {
    window.render_begin();
    Box2 mask(Vec2(), window.size());

    geometry.queue_box(box, Color::Red(), 5, Color::Black(), 0, mask);
    text.queue_text(
        Vec2(400, 100),
        "Hello",
        Font::DejaVuSans,
        20,
        Color::Blue(),
        LengthWrap(),
        mask);
    text.queue_text(
        Vec2(400, 150),
        "Hello",
        Font::DejaVuSansMono,
        40,
        Color::Blue(),
        LengthWrap(),
        mask);
    text.queue_text(
        Vec2(400, 220),
        "Hello",
        Font::DejaVuSerif,
        60,
        Color::Blue(),
        LengthWrap(),
        mask);

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
