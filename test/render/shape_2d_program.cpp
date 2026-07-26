#include "datagui/render/program/shape_2d_program.hpp"
#include "datagui/visual/window.hpp"

int main() {
  using namespace dgui;

  Window window;
  window.open("shape_2d_program", 500, 500);

  Shape2dProgram program;
  program.init();

  std::vector<Shape2dInstance> instances;

  // Baseline rect (centered in bottom-left corner)
  instances.push_back(
      Shape2dInstance::rect(
          Vec2(0, 0),
          0.f,
          Vec2(200, 200),
          Color::Red(),
          2,
          Color::Black()));

  // Rounded box
  instances.push_back(
      Shape2dInstance::box(
          Box2(Vec2(40, 360), Vec2(200, 440)),
          Color::Green(),
          3,
          Color::Black(),
          20));

  // Rotated rect
  instances.push_back(
      Shape2dInstance::rect(
          Vec2(260, 400),
          0.4f,
          Vec2(120, 80),
          Color::Blue(),
          3,
          Color::Black()));

  // Circle
  instances.push_back(
      Shape2dInstance::circle(
          Vec2(430, 400),
          50,
          Color(1.f, 0.6f, 0.f, 1.f),
          4,
          Color::Black()));

  // Rotated ellipse
  instances.push_back(
      Shape2dInstance::ellipse(
          Vec2(90, 250),
          0.3f,
          Vec2(70, 40),
          Color(0.6f, 0.2f, 0.8f, 1.f),
          3,
          Color::Black()));

  // Line with rounded ends
  instances.push_back(
      Shape2dInstance::line(
          Vec2(180, 210),
          Vec2(330, 300),
          12,
          Color::Black(),
          true));

  // Capsule
  instances.push_back(
      Shape2dInstance::capsule(
          Vec2(390, 210),
          Vec2(460, 300),
          22,
          Color::Gray(0.5f),
          3,
          Color::Black()));

  // Opacity: three overlapping translucent circles (alpha 0.5)
  instances.push_back(
      Shape2dInstance::circle(
          Vec2(210, 110),
          55,
          Color(1.f, 0.f, 0.f, 0.5f),
          0,
          Color::Clear()));
  instances.push_back(
      Shape2dInstance::circle(
          Vec2(265, 110),
          55,
          Color(0.f, 1.f, 0.f, 0.5f),
          0,
          Color::Clear()));
  instances.push_back(
      Shape2dInstance::circle(
          Vec2(237, 155),
          55,
          Color(0.f, 0.f, 1.f, 0.5f),
          0,
          Color::Clear()));

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
    program.draw(instances.data(), instances.size(), PV);

    window.render_end();
    window.poll_events();
  }
}
