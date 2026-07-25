#include "datagui/render/shape_2d_program.hpp"
#include "datagui/visual/window.hpp"

int main() {
  using namespace dgui;

  Window window;
  window.open("shape_program", 500, 500);

  Shape2dProgram program;
  program.init();

  std::vector<Shape2dInstance> instances;
  instances.push_back(
      Shape2dInstance::rect(
          Vec2(0, 0),
          0.f,
          Vec2(200, 200),
          Color::Red(),
          2,
          Color::Black()));

  while (window.running()) {
    window.render_begin();

    Mat3 PV = Mat3{
        {2.f / window.size().x, 0.f, -1.f},
        {0.f, 2.f / window.size().y, -1.f},
        {0.f, 0.f, 1.f}};

    program.bind();
    program.draw(instances.data(), instances.size(), PV);

    window.render_end();
    window.poll_events();
  }
}
