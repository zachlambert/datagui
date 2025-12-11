#include <datagui/visual/image_shader.hpp>
#include <datagui/visual/shape_shader.hpp>
#include <datagui/visual/text_shader.hpp>
#include <datagui/visual/window.hpp>
#include <iostream>

int main() {
  using namespace datagui;

  Window window;
  ShapeShader shape_shader;
  TextShader text_shader;
  ImageShader image_shader;
  auto font_manager = std::make_shared<FontManager>();

  shape_shader.init();
  text_shader.init(font_manager);
  image_shader.init();

  struct Pixel {
    std::uint8_t r, g, b, a;
  };
  std::vector<Pixel> pixels(100 * 100);
  for (std::size_t i = 0; i < 100; i++) {
    for (std::size_t j = 0; j < 100; j++) {
      pixels[i * 100 + j].r = (float(j) / 100) * 255;
      pixels[i * 100 + j].g = (float(i) / 100) * 255;
      pixels[i * 100 + j].b = 0;
      pixels[i * 100 + j].a = 255;
    }
  }
  Image image;
  image.load(100, 100, pixels.data());

  while (window.running()) {
    window.render_begin();
    Box2 mask = Box2(Vec2(), window.size());

    text_shader.queue_text(
        Vec2(200, 220),
        0,
        "Hello",
        Font::DejaVuSerif,
        60,
        Color::Blue(),
        LengthWrap());
    text_shader.draw(window.size());

    shape_shader.queue_box(
        Box2(Vec2(50, 50), Vec2(300, 300)),
        Color::Red(),
        0,
        20,
        Color::Black(),
        mask);
    shape_shader.draw(window.size());

    text_shader.queue_text(
        Vec2(200, 100),
        0,
        "Hello",
        Font::DejaVuSans,
        20,
        Color::Blue(),
        LengthWrap());
    text_shader.queue_text(
        Vec2(200, 150),
        0,
        "Hello",
        Font::DejaVuSansMono,
        40,
        Color::Blue(),
        LengthWrap());
    text_shader.draw(window.size());

    shape_shader.queue_box(
        Box2(Vec2(50, 350), Vec2(150, 450)),
        Color::Green(0.2),
        10,
        10,
        Color::Black(),
        mask);
    shape_shader.queue_box(
        Box2(Vec2(100, 400), Vec2(200, 500)),
        Color::Green(0.5),
        30,
        10,
        Color::Green(0.2),
        mask);
    shape_shader.queue_box(
        Box2(Vec2(150, 450), Vec2(250, 550)),
        Color::Green(0.8),
        50,
        10,
        Color::Green(0.5),
        mask);
    shape_shader.draw(window.size());

    shape_shader.queue_rect(
        Vec2(500, 100),
        M_PI / 8,
        Vec2(100, 50),
        Color::Hsl(70, 1, 0.3),
        0,
        0,
        Color::Black(),
        mask);
    shape_shader.queue_capsule(
        Vec2(500, 150),
        Vec2(600, 100),
        30,
        Color::Clear(),
        10,
        Color::Hsl(140, 1, 0.7),
        mask);
    shape_shader.queue_circle(
        Vec2(500, 300),
        100,
        Color::Hsl(220, 1, 0.7),
        50,
        Color::Hsl(220, 1, 0.3),
        mask);
    shape_shader.queue_ellipse(
        Vec2(500, 400),
        -M_PI / 4,
        100,
        50,
        Color::Hsl(280, 1, 0.7),
        5,
        Color::Hsl(280, 1, 0.3),
        mask);
    shape_shader.queue_ellipse(
        Vec2(550, 450),
        -M_PI / 4,
        100,
        50,
        Color::Hsl(30, 1, 0.5, 0.5),
        10,
        Color::Hsl(30, 1, 0.5),
        mask);
    shape_shader.queue_capsule(
        Vec2(700, 100),
        Vec2(700, 500),
        5,
        Color::Gray(0.5),
        0,
        Color::Black(),
        mask);
    shape_shader.draw(window.size());

    image_shader
        .queue_image(image, Vec2(750, 200), M_PI / 4, Vec2::uniform(200));
    image_shader.draw(window.size());

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
