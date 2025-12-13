#include <datagui/visual/shape_3d_shader.hpp>
#include <datagui/visual/window.hpp>

int main() {
  using namespace datagui;

  Window window;
  Shape3dShader shape_shader;
  shape_shader.init();

  Camera3d camera;
  camera.direction.x = std::cos(M_PI / 4);
  camera.direction.z = -std::sin(M_PI / 4);
  camera.position.x = -8.0;
  camera.position.z = 6;
  camera.fov_degrees = 90;
  camera.clipping_min = 0.001;
  camera.clipping_max = 1000;

  while (window.running()) {
    window.render_begin();

    shape_shader.queue_box(
        Vec3(),
        Euler(M_PI / 4, 0, 0),
        Vec3::uniform(1),
        Color::Red());

    shape_shader
        .queue_cylinder(Vec3(0, 2, 0), Vec3(0, 0, 1), 0.5, 2, Color::Green());

    shape_shader.queue_sphere(Vec3(0, -2, 0), 1.5, Color::Blue());

    shape_shader.draw(window.size(), camera);
    shape_shader.clear();

    window.render_end();
    window.poll_events();
  }
}
