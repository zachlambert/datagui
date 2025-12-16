#include <datagui/visual/shape_2d_shader.hpp>
#include <datagui/visual/window.hpp>

int main() {
  using namespace datagui;

  Window window;
  Shape2dShader shader;
  shader.init();

  int i = 0;
  while (window.running()) {
    window.render_begin();

    shader.queue_masked_box(
        Box2(Vec2(), window.size()),
        Box2(Vec2(10, 10), Vec2(200, 100)),
        Color::Red());

    shader.queue_masked_box(
        Box2(Vec2(), window.size()),
        Box2(Vec2(10, 110), Vec2(200, 200)),
        Color::Red(),
        0,
        Color(),
        20);

    shader.queue_masked_box(
        Box2(Vec2(), window.size()),
        Box2(Vec2(210, 10), Vec2(400, 100)),
        Color::Green(),
        10,
        Color::Black());

    shader.queue_masked_box(
        Box2(Vec2(), window.size()),
        Box2(Vec2(210, 110), Vec2(400, 200)),
        Color::Green(),
        10,
        Color::Black(),
        20);

    shader.queue_masked_box(
        Box2(Vec2(440, 60), Vec2(620, 200)),
        Box2(Vec2(410, 10), Vec2(600, 100)),
        Color::Blue(),
        5,
        Color::Gray(0.5));

    shader.queue_masked_box(
        Box2(Vec2(440, 160), Vec2(620, 300)),
        Box2(Vec2(410, 110), Vec2(600, 200)),
        Color::Blue(),
        5,
        Color::Gray(0.5),
        20);

    Camera2d camera;
    camera.position = window.size() / 2 +
                      50.f * (Rot2(2 * M_PI * i / 100).mat() * Vec2::unit_x());
    camera.angle = (M_PI / 10) * (-0.5 * std::sin(2 * M_PI * i / 200));
    i++;
    camera.width = window.size().x;

    shader.draw(window.viewport(), camera);
    shader.clear();

    window.render_end();
    window.poll_events();
  }
}
