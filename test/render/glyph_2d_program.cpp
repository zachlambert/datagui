#include "datagui/render/glyph_2d_program.hpp"
#include "datagui/font.hpp"
#include "datagui/render/font_atlas.hpp"
#include "datagui/render/font_program.hpp"
#include "datagui/visual/window.hpp"

int main() {
  using namespace dgui;

  Window window;
  window.open("glyph_2d_program", 500, 500);

  FontProgram font_program;
  font_program.init();

  FontAtlas atlas(font_program, lookup_font(Font::DejaVuSans), 24);

  Glyph2dProgram program;
  program.init();
  std::vector<Glyph2dVertex> vertices;

  atlas.add_glyphs(vertices, Vec2(100, 100), 0, Vec2::ones(), "Testing");

  while (window.running()) {
    window.render_begin();

    Mat3 PV = Mat3{
        {2.f / window.size().x, 0.f, -1.f},
        {0.f, 2.f / window.size().y, -1.f},
        {0.f, 0.f, 1.f}};

    program.bind();
    program.draw(
        atlas.texture(),
        Color::Black(),
        vertices.data(),
        vertices.size(),
        PV);

    window.render_end();
    window.poll_events();
  }
}
