#include <datagui/visual/renderer.hpp>
#include <datagui/visual/window.hpp>
#include <iostream>

int main() {
  using namespace datagui;

  Window window;
  auto font_manager = std::make_shared<FontManager>();
  Renderer renderer;
  renderer.init(font_manager);

  Box2 box;
  box.lower = Vec2(50, 50);
  box.upper = Vec2(300, 300);

  while (window.running()) {
    window.render_begin();
    renderer.render_begin(window.size());

    renderer.queue_text(
        Vec2(200, 220),
        "Hello",
        Font::DejaVuSerif,
        60,
        Color::Blue());
    renderer.queue_box(
        Box2(Vec2(50, 50), Vec2(300, 300)),
        Color::Red(),
        5,
        Color::Black());
    renderer.queue_text(
        Vec2(200, 100),
        "Hello",
        Font::DejaVuSans,
        20,
        Color::Blue());
    renderer.queue_text(
        Vec2(200, 150),
        "Hello",
        Font::DejaVuSansMono,
        40,
        Color::Blue(),
        LengthWrap());

    renderer.queue_box(Box2(Vec2(50, 350), Vec2(150, 450)), Color::Green(0.8));
    renderer.queue_box(Box2(Vec2(100, 400), Vec2(200, 500)), Color::Green(0.5));
    renderer.queue_box(Box2(Vec2(150, 450), Vec2(250, 550)), Color::Green(0.2));

    renderer.render_end();
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
