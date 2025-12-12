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

    for (float x = -5; x <= 5; x += 1) {
      for (float y = -5; y <= 5; y += 1) {
        shape_shader.queue_box(
            Vec3(x, y, -5 * std::exp(-std::hypot(x / 5, y / 5))),
            Rot3(),
            Vec3::uniform(0.8),
            Color::Hsl(
                180.f * std::atan2(y / 5, x / 5) / M_PIf,
                std::hypot(x / 10, y / 10),
                0.5,
                1));
      }
    }
    shape_shader.draw(window.size(), camera);
    shape_shader.clear();

    window.render_end();
    window.poll_events();
  }
}
