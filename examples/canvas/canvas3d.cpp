#include "datagui/viewport/canvas3d.hpp"
#include "datagui/gui.hpp"

int main() {
  dgui::Gui gui;
  gui.open();
  using dgui::Color;
  using dgui::Euler;
  using dgui::Rot3;
  using dgui::Vec3;

  dgui::PointCloud point_cloud;
  {
    struct Point {
      Vec3 position;
      Vec3 color;
    };
    std::vector<Point> points;
    for (float theta = -M_PI / 2; theta <= M_PI / 2; theta += 0.1 / 8) {
      for (float z = 0; z <= 5; z += 0.1) {
        Point point;
        point.position.x = 8 * std::cos(theta);
        point.position.y = 8 * std::sin(theta);
        point.position.z = z + std::sin(theta * 10);
        point.color = Vec3(0.2 * z, 1 - 0.2 * z, 0);
        points.push_back(point);
      }
    }
    point_cloud.load_colored_points(
        points.data(),
        points.size(),
        offsetof(Point, position),
        offsetof(Point, color),
        sizeof(Point));
  }

  Color bg_color = Color::Hsl(220, 0.4, 0.85);
  float box_opacity = gui.variable<float>(0.8);
  float box_size = 2.0;

  while (gui.poll()) {
    gui.args().horizontal().align_top();
    gui.group();
    DGUI_SCOPE(gui);

    auto& canvas = gui.canvas3d(512, 512);
    {
      DGUI_SCOPE(gui);
      canvas.bg_color(bg_color);
      canvas.grid(10, 10);
      canvas.box(
          Vec3(0, 0, 1),
          Rot3(),
          Vec3::uniform(box_size),
          Color(1, 0, 0, box_opacity));
      canvas.box(
          Vec3(3, 0, 1),
          Rot3(Euler(M_PI / 4, 0, 0)),
          Vec3::uniform(2),
          Color(0, 1, 0, 0.5));
      canvas.box(
          Vec3(0, 3, 1),
          Rot3(Euler(0, M_PI / 4, 0)),
          Vec3::uniform(2),
          Color(0, 0, 1, 0.2));

      canvas.cylinder(
          Vec3(0, -3, 0),
          Vec3(std::sin(M_PI / 4), 0, std::cos(M_PI / 4)),
          0.5,
          5,
          Color::Hsl(50, 1, 0.5));

      canvas.sphere(Vec3(-3, 0, 1), 1, Color::Hsl(300, 1, 0.5));

      canvas
          .cone(Vec3(-3, -3, 0), Vec3(0, 0, 1), 1, 2, Color::Hsl(200, 1, 0.5));

      canvas.axes(Vec3(-5, -5, 0), Rot3(), 2);

      canvas.point_cloud(point_cloud, Vec3(), Rot3(), 0.08);
    }

    gui.args().grid(-1, 2);
    gui.group();
    {
      DGUI_SCOPE(gui);

      gui.text_box("Bg Color");
      gui.args().always();
      gui.color_picker_v(bg_color);

      gui.text_box("Red Box Opacity");
      gui.args().always();
      gui.slider_v(box_opacity, 0.f, 1.f);

      gui.text_box("Red Box Size");
      gui.args().always();
      gui.slider_v(box_size, 0.f, 10.f);
    }
  }
}
