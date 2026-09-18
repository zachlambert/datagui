#include "datagui/font.hpp"
#include "datagui/render/executor.hpp"
#include "datagui/render/program/font_program.hpp"
#include "datagui/render/program_registry.hpp"
#include "datagui/render/state/draw_list.hpp"
#include "datagui/render/window.hpp"

int main() {
  using namespace dgui;

  Window window;
  window.open("render", 500, 500);

  ProgramRegistry registry;
  registry.init();

  FontProgram font_program;
  font_program.init();
  FontAtlas font_atlas(font_program, lookup_font(Font::DejaVuSans), 24);

  DrawList dl;
  dl.new_group(Box2(Vec2(0, 0), Vec2(500, 500)));
  dl.draw_box(
      Box2(Vec2(0, 0), Vec2(500, 500)),
      Color::Gray(0.9),
      0,
      Color::Black(),
      0);
  dl.draw_box(
      Box2(Vec2(100, 200), Vec2(200, 280)),
      Color::Red(),
      0,
      Color::Black(),
      0);
  dl.draw_text(
      font_atlas,
      Vec2(150, 200),
      Color::Black(),
      LengthWrap(),
      "Hello there");

  dl.new_group(Box2(Vec2(0, 0), Vec2(500, 500)));

  dl.draw_box(
      Box2(Vec2(250, 200), Vec2(300, 280)),
      Color::Red(),
      0,
      Color::Black(),
      0);
  dl.draw_box(
      Box2(Vec2(100, 100), Vec2(1000, 150)),
      Color::Blue(),
      20,
      Color::Black(),
      20);

  auto scene_2d = std::make_shared<Scene2d>();
  {
    Box2 viewport = Box2(Vec2(50, 300), Vec2(250, 500));
    Camera2d camera;
    camera.size = Vec2(10, 10);
    camera.position = Vec2(5, 5);
    camera.angle = M_PI / 8;
    dl.draw_scene_2d(viewport, camera, scene_2d);
  }

  scene_2d->bg_color = Color::Hsl(0, 0.5f, 0.8f);
  scene_2d->draw_circle(
      Vec2(5, 5),
      5,
      Color::Hsl(180, 0.5f, 0.9f),
      0.1,
      Color::Black());
  scene_2d
      ->draw_rect(Vec2(2, 2), 0, Vec2(2, 2), Color::Green(), 0, Color::Black());
  scene_2d->draw_text(
      font_atlas,
      Vec2(3, 3),
      0,
      Vec2::uniform(2.f / 24),
      Color::Black(),
      LengthWrap(),
      "TEXT");

  auto scene_3d = std::make_shared<Scene3d>();
  {
    Box2 viewport = Box2(Vec2(350, 300), Vec2(550, 500));
    Camera3d camera;
    camera.position = Vec3(-2, 0, 2);
    camera.direction = Vec3(1, 0, -1) / std::sqrt(2);
    camera.fov = Vec2::uniform(70.f * M_PI / 180);
    camera.clipping_min = 1e-2f;
    camera.clipping_max = 1e2f;
    dl.draw_scene_3d(viewport, camera, scene_3d);
  }

  scene_3d->bg_color = Color::Black();
  scene_3d->draw_box(Vec3(0, 0, 0), Rot3(), Vec3::ones(), Color::Red());

  Executor executor;

  while (window.running()) {
    window.render_begin();
    executor.draw(window.size(), registry, dl);
    window.render_end();
    window.poll_events();
  }
}
