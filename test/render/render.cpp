#include "datagui/font.hpp"
#include "datagui/render/draw_list.hpp"
#include "datagui/render/draw_queue.hpp"
#include "datagui/render/executor.hpp"
#include "datagui/render/program/font_program.hpp"
#include "datagui/render/program_registry.hpp"
#include "datagui/visual/window.hpp"

int main() {
  using namespace dgui;

  Window window;
  window.open("render", 500, 500);

  ProgramRegistry registry;
  registry.init();

  FontProgram font_program;
  font_program.init();
  FontAtlas font_atlas(font_program, lookup_font(Font::DejaVuSans), 24);

  DrawQueue queue;
  queue.begin_group(Box2(Vec2(0, 0), Vec2(500, 500)));
  queue.draw_box(
      Box2(Vec2(0, 0), Vec2(500, 500)),
      Color::Gray(0.9),
      0,
      Color::Black(),
      0);
  queue.draw_box(
      Box2(Vec2(100, 200), Vec2(200, 280)),
      Color::Red(),
      0,
      Color::Black(),
      0);
  queue.draw_text(
      font_atlas,
      Vec2(150, 200),
      0,
      Vec2::ones(),
      Color::Black(),
      LengthWrap(),
      "Hello there");
  queue.draw_box(
      Box2(Vec2(250, 200), Vec2(300, 280)),
      Color::Red(),
      0,
      Color::Black(),
      0);
  queue.draw_box(
      Box2(Vec2(100, 100), Vec2(1000, 150)),
      Color::Blue(),
      20,
      Color::Black(),
      20);

  Box2 camera_viewport = Box2(Vec2(50, 300), Vec2(250, 500));
  Camera2d camera;
  camera.size = Vec2(10, 10);
  camera.position = Vec2(5, 5);
  camera.angle = M_PI / 8;
  queue.begin_scene_2d(camera_viewport, camera);

  queue.draw_box(
      Box2(Vec2(0, 0), Vec2(10, 10)),
      Color::Hsl(180, 0.5f, 0.9f),
      0.1,
      Color::Black(),
      5);
  queue.draw_box(
      Box2(Vec2(1, 1), Vec2(3, 3)),
      Color::Green(),
      0,
      Color::Black(),
      0);
  queue.draw_text(
      font_atlas,
      Vec2(3, 3),
      0,
      Vec2::uniform(2.f / 24),
      Color::Black(),
      LengthWrap(),
      "TEXT");

  DrawList draw_list = queue.flush();
  Executor executor;

  while (window.running()) {
    window.render_begin();
    executor.draw(window.size(), registry, draw_list);
    window.render_end();
    window.poll_events();
  }
}
