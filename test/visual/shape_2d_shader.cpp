#include <datagui/visual/shape_2d_shader.hpp>
#include <datagui/visual/window.hpp>

int main() {
  using namespace datagui;

  Window window;
  Shape2dShader shader;
  shader.init();

  while (window.running()) {
    window.render_begin();

    // queue_masked_box

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

    // queue_rect

    shader.queue_rect(
        Vec2(100, 300),
        M_PI / 8,
        Vec2(150, 30),
        Color::Hsl(100, 0.5, 0.5),
        2);

    shader.queue_circle(Vec2(180, 300), 60, Color::Hsl(220, 0.5, 0.5, 0.5), 2);

    shader.queue_ellipse(
        Vec2(400, 300),
        -M_PI / 8,
        Vec2(150, 40),
        Color::Hsl(300, 1, 0.5),
        10);

    shader.queue_capsule(
        Vec2(450, 400),
        Vec2(350, 500),
        40,
        Color::Hsl(40, 1, 0.5),
        10);

    shader.queue_line(
        Vec2(660, 200),
        Vec2(660, 500),
        20,
        Color::Hsl(300, 1, 0.5));

    shader.queue_line(
        Vec2(700, 200),
        Vec2(700, 500),
        20,
        Color::Hsl(260, 1, 0.5),
        false);

    // draw

    Camera2d camera;
    camera.position = window.size() / 2;
    camera.angle = 0;
    camera.size = window.size();

    shader.draw(window.viewport(), camera);
    shader.clear();

    window.render_end();
    window.poll_events();
  }
}
