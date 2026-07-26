#include "datagui/asset/image.hpp"
#include "datagui/color.hpp"
#include "datagui/geometry/rot.hpp"
#include "datagui/render/program/image_2d_program.hpp"
#include "datagui/visual/window.hpp"
#include <GL/glew.h>
#include <cmath>
#include <cstdint>
#include <vector>

dgui::Image make_image(
    std::size_t w,
    std::size_t h,
    dgui::Vec2 n,
    const dgui::Color& a,
    const dgui::Color& b) {
  n /= n.length();

  struct Pixel {
    std::uint8_t r, g, b, a;
  };
  std::vector<Pixel> pixels(w * h);

  float half_width = float(w) / 2;
  float half_height = float(h) / 2;
  for (std::size_t i = 0; i < h; i++) {
    for (std::size_t j = 0; j < w; j++) {
      dgui::Vec2 pos;
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

  dgui::Image image;
  image.load(w, h, pixels.data());
  return image;
}

int main() {
  using namespace dgui;

  Window window;
  window.open("image_program", 500, 500);

  Image2dProgram program;
  program.init();

  Image image_1 = make_image(128, 128, {1, 1}, Color::Red(), Color::Blue());
  Image image_2 = make_image(
      128,
      128,
      {2, -1},
      Color::Hsl(160, 1, 0.5),
      Color::Hsl(200, 1, 0.5, 0.4));

  Mat3 M1 = {{100.f, 0.f, 100.f}, {0.f, 100.f, 100.f}, {0.f, 0.f, 1.f}};
  Mat3 M2 = {{100.f, 0.f, 300.f}, {0.f, 100.f, 100.f}, {0.f, 0.f, 1.f}};

  while (window.running()) {
    window.render_begin();

    glViewport(0, 0, (int)window.size().x, (int)window.size().y);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Mat3 PV = Mat3{
        {2.f / window.size().x, 0.f, -1.f},
        {0.f, 2.f / window.size().y, -1.f},
        {0.f, 0.f, 1.f}};

    program.bind();
    program.draw(image_1.texture(), M1, PV);
    program.draw(image_2.texture(), M2, PV);

    window.render_end();
    window.poll_events();
  }
}
