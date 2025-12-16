#include <datagui/visual/text_2d_shader.hpp>
#include <datagui/visual/window.hpp>

int main() {
  using namespace datagui;

  Window window;
  Text2dShader shader;
  auto fm = std::make_shared<FontManager>();
  shader.init(fm);

  while (window.running()) {
    window.render_begin();

    shader.queue_masked_text(
        Box2(Vec2(), window.size()),
        Vec2(100, 100),
        "hello",
        Font::DejaVuSans,
        40,
        Color::Blue());

    shader.queue_text(
        Vec2(250, 100),
        M_PI / 4,
        "rotated?",
        Font::DejaVuSansMono,
        24,
        Color::Black());

    shader.queue_masked_text(
        Box2(Vec2(110, 180), Vec2(300, 210)),
        Vec2(100, 200),
        "hello",
        Font::DejaVuSerif,
        30,
        Color::Red());

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
