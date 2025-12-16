#include <datagui/color.hpp>
#include <datagui/visual/image_shader.hpp>
#include <datagui/visual/window.hpp>

datagui::Image make_image(
    std::size_t w,
    std::size_t h,
    datagui::Vec2 n,
    const datagui::Color& a,
    const datagui::Color& b) {
  n /= n.length();

  struct Pixel {
    std::uint8_t r, g, b, a;
  };
  std::vector<Pixel> pixels(w * h);

  float half_width = float(w) / 2;
  float half_height = float(h) / 2;
  for (std::size_t i = 0; i < h; i++) {
    for (std::size_t j = 0; j < w; j++) {
      datagui::Vec2 pos;
      pos.x = (j - half_width) / half_width;
      pos.y = (i - half_height) / half_height;

      float s = 0.5 * (1 + std::cos(pos.dot(n) * 10));
      auto& pixel = pixels[i * w + j];
      pixel.r = 255 * ((1 - s) * a.r + s * b.r);
      pixel.g = 255 * ((1 - s) * a.g + s * b.g);
      pixel.b = 255 * ((1 - s) * a.b + s * b.b);
      pixel.a = 255 * ((1 - s) * a.a + s * b.a);
    }
  }

  datagui::Image image;
  image.load(w, h, pixels.data());
  return std::move(image);
}

int main() {
  using namespace datagui;

  Window window;
  ImageShader shader;
  shader.init();

  auto image_1 = make_image(128, 128, {1, 1}, Color::Red(), Color::Blue());
  auto image_2 = make_image(
      128,
      128,
      {2, -1},
      Color::Hsl(160, 1, 0.5),
      Color::Hsl(200, 1, 0.5, 0.4));

  auto image_3 = make_image(
      128,
      128,
      {0, 1},
      Color::Hsl(270, 1, 0.2),
      Color::Hsl(330, 1, 0.8));

  while (window.running()) {
    window.render_begin();

    shader.queue_image(image_1, Vec2(100, 100), 0, Vec2(200, 200));
    shader.queue_image(image_2, Vec2(150, 150), 0, Vec2(200, 200));
    shader.queue_masked_image(
        Box2(Vec2(400, 300), Vec2(600, 500)),
        image_3,
        Vec2(300, 250),
        Vec2(200, 200));

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
