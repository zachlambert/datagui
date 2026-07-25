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

  // One atlas per font
  FontAtlas sans(font_program, lookup_font(Font::DejaVuSans), 24);
  FontAtlas serif(font_program, lookup_font(Font::DejaVuSerif), 24);
  FontAtlas mono(font_program, lookup_font(Font::DejaVuSansMono), 24);

  Glyph2dProgram program;
  program.init();

  // Each item is drawn with a single atlas texture and a single color, so
  // varying font or color means a separate item; angle/scale vary per call.
  struct TextItem {
    unsigned int texture;
    Color color;
    std::vector<Glyph2dVertex> vertices;
  };
  std::vector<TextItem> items;

  auto add_text = [&](FontAtlas& atlas,
                      const Color& color,
                      const Vec2& origin,
                      double angle,
                      const Vec2& scale,
                      const std::string& text,
                      Length width = LengthWrap()) {
    TextItem& item = items.emplace_back();
    item.texture = atlas.texture();
    item.color = color;
    atlas.add_glyphs(item.vertices, origin, angle, scale, text, width);
  };

  // Multiple fonts, one color each
  add_text(sans, Color::Black(), Vec2(30, 470), 0, Vec2::ones(), "DejaVu Sans");
  add_text(serif, Color::Red(), Vec2(30, 430), 0, Vec2::ones(), "DejaVu Serif");
  add_text(mono, Color::Blue(), Vec2(30, 390), 0, Vec2::ones(), "DejaVu Mono");

  // Same font, multiple colors
  add_text(sans, Color::Green(), Vec2(30, 350), 0, Vec2::ones(), "green");
  add_text(sans, Color::Gray(0.5f), Vec2(140, 350), 0, Vec2::ones(), "gray");

  // Scales: uniform enlargement and a vertical stretch
  add_text(sans, Color::Black(), Vec2(30, 300), 0, Vec2(2.f, 2.f), "Big x2");
  add_text(mono, Color::Blue(), Vec2(30, 240), 0, Vec2(1.f, 2.f), "Tall");

  // Angles (radians), including a rotated + scaled item
  add_text(sans, Color::Black(), Vec2(300, 240), 0.5, Vec2::ones(), "Rotated");
  add_text(
      serif,
      Color::Red(),
      Vec2(260, 130),
      -0.4,
      Vec2(1.5f, 1.5f),
      "Angled");

  // Fixed-width wrapping
  add_text(
      mono,
      Color::Black(),
      Vec2(30, 170),
      0,
      Vec2::ones(),
      "wrapped mono text",
      LengthFixed(120));

  while (window.running()) {
    window.render_begin();

    glViewport(0, 0, (int)window.size().x, (int)window.size().y);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Mat3 PV = Mat3{
        {2.f / window.size().x, 0.f, -1.f},
        {0.f, 2.f / window.size().y, -1.f},
        {0.f, 0.f, 1.f}};

    for (const auto& item : items) {
      program.bind();
      program.draw(
          item.texture,
          item.color,
          item.vertices.data(),
          item.vertices.size(),
          PV);
    }

    window.render_end();
    window.poll_events();
  }
}
