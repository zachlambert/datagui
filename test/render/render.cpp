#include "datagui/render/draw_builder.hpp"
#include "datagui/render/draw_list.hpp"
#include "datagui/render/executor.hpp"
#include "datagui/render/program_registry.hpp"
#include "datagui/visual/window.hpp"

int main() {
  using namespace dgui;

  Window window;
  window.open("render", 500, 500);

  ProgramRegistry registry;
  registry.init();

  DrawBuilder builder;
  Executor executor;

  while (window.running()) {
    window.render_begin();

    builder.begin_group(0, Box2(Vec2(0, 0), Vec2(500, 500)));
    builder.queue_box(
        Box2(Vec2(0, 0), Vec2(500, 500)),
        Color::Gray(0.9),
        0,
        Color::Black(),
        0);
    builder.queue_box(
        Box2(Vec2(100, 100), Vec2(1000, 150)),
        Color::Blue(),
        20,
        Color::Black(),
        20);

    DrawList draw_list = builder.flush();
    executor.draw(window.size(), registry, draw_list);

    window.render_end();
    window.poll_events();
  }
}
