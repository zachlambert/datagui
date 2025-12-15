#include <datagui/visual/masked_box_shader.hpp>
#include <datagui/visual/window.hpp>

int main() {
  using namespace datagui;

  Window window;
  MaskedBoxShader shader;
  shader.init();

  while (window.running()) {
    window.render_begin();

    shader.queue_box(
        Box2(Vec2(), window.size()),
        Box2(Vec2(10, 10), Vec2(100, 100)),
        Color::Red());

    shader.queue_box(
        Box2(Vec2(), window.size()),
        Box2(Vec2(10, 110), Vec2(100, 200)),
        Color::Red(),
        0,
        Color(),
        20);

    shader.queue_box(
        Box2(Vec2(), window.size()),
        Box2(Vec2(110, 10), Vec2(200, 100)),
        Color::Green(),
        10,
        Color::Black());

    shader.queue_box(
        Box2(Vec2(), window.size()),
        Box2(Vec2(110, 110), Vec2(200, 200)),
        Color::Green(),
        10,
        Color::Black(),
        20);

    shader.queue_box(
        Box2(Vec2(240, 60), Vec2(400, 200)),
        Box2(Vec2(210, 10), Vec2(300, 100)),
        Color::Blue());

    shader.queue_box(
        Box2(Vec2(240, 160), Vec2(400, 300)),
        Box2(Vec2(210, 110), Vec2(300, 200)),
        Color::Blue(),
        0,
        Color(),
        20);

    shader.draw(window.viewport());
    shader.clear();

    window.render_end();
    window.poll_events();
  }
}
