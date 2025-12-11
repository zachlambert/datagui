#include <datagui/visual/shape_3d_shader.hpp>
#include <datagui/visual/window.hpp>
#include <iostream>

int main() {
  using namespace datagui;

  Window window;
  Shape3dShader shape_shader;
  shape_shader.init();

  Camera3d camera;
  camera.direction.x = std::cos(M_PI / 6);
  camera.direction.z = -std::sin(M_PI / 6);
  camera.position.x = -3.0;
  camera.position.z = 2;
  camera.fov_horizontal_degrees = 90;
  camera.fov_vertical_degrees = 90;
  camera.clipping_min = 0.01;
  camera.clipping_max = 100;

  while (window.running()) {
    window.render_begin();

    shape_shader.queue_box(Vec3(), Rot3(), Vec3::ones(), Color::Red());
    shape_shader.queue_box(Vec3(2, 0, 0), Rot3(), Vec3::ones(), Color::Red());
    shape_shader.queue_box(Vec3(0, 2, 0), Rot3(), Vec3::ones(), Color::Red());
    shape_shader.queue_box(Vec3(0, -2, 0), Rot3(), Vec3::ones(), Color::Red());
    shape_shader.draw(window.size(), camera);

    window.render_end();

    window.poll_events();
    for (const auto& event : window.mouse_events()) {
      switch (event.action) {
      case MouseAction::Press:
        std::cout << "Mouse press: " << int(event.button) << std::endl;
        break;
      case MouseAction::Release:
        std::cout << "Mouse release: " << int(event.button) << std::endl;
        break;
      default:
        break;
      }
    }
  }
}
