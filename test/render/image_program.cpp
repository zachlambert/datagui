#include "datagui/asset/image.hpp"
#include "datagui/color.hpp"
#include "datagui/geometry/rot.hpp"
#include "datagui/render/image_2d_program.hpp"
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

// Build the 6 vertices for an image quad, mirroring DrawBuilder::queue_image.
std::vector<dgui::Image2dVertex> image_quad(
    const dgui::Vec2& position,
    float angle,
    const dgui::Vec2& size) {
  using namespace dgui;
  Mat2 rot = Rot2(angle).mat();
  Vec2 lower_left = position;
  Vec2 lower_right = position + rot * Vec2(size.x, 0);
  Vec2 upper_left = position + rot * Vec2(0, size.y);
  Vec2 upper_right = position + rot * size;

  // UV V flipped so the image's top row maps to the top of the quad.
  return {
      {lower_left, Vec2(0, 1)},
      {lower_right, Vec2(1, 1)},
      {upper_left, Vec2(0, 0)},
      {lower_right, Vec2(1, 1)},
      {upper_right, Vec2(1, 0)},
      {upper_left, Vec2(0, 0)}};
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

  auto verts_1 = image_quad(Vec2(100, 100), 0, Vec2(200, 200));
  auto verts_2 = image_quad(Vec2(150, 150), 0, Vec2(200, 200));

  // The old queue_masked_image is now expressed as scissor-based clipping.
  const Box2 mask(Vec2(400, 300), Vec2(600, 500));

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
    program.draw(image_1.texture(), verts_1.data(), verts_1.size(), PV);
    program.bind();
    program.draw(image_2.texture(), verts_2.data(), verts_2.size(), PV);

    window.render_end();
    window.poll_events();
  }
}
