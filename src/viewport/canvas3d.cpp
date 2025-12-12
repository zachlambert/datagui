#include "datagui/viewport/canvas3d.hpp"

namespace datagui {

Canvas3d::Canvas3d() {
  camera.direction = Rot3(Euler(0, M_PI / 6, M_PI / 4)).mat() * Vec3(1, 0, 0);
  camera.position.x = -7;
  camera.position.y = -7;
  camera.position.z = 5;
  camera.fov_degrees = 70;
  camera.clipping_min = 0.001;
  camera.clipping_max = 1000;
}

void Canvas3d::box(
    const Vec3& position,
    const Rot3& orientation,
    const Vec3& size,
    const Color& color) {
  shape_shader.queue_box(position, orientation, size, color);
}

void Canvas3d::grid(std::size_t size, float width) {
  shape_shader.queue_box(
      Vec3(0, 0, -0.05),
      Rot3(),
      Vec3(width, width, 0.1),
      Color::Gray(0.8));
}

void Canvas3d::begin() {
  //
}

void Canvas3d::end() {
  bind_framebuffer();
  shape_shader.draw(framebuffer_size(), camera);
  unbind_framebuffer();
}

void Canvas3d::impl_init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontManager>& fm) {
  shape_shader.init();
}

void Canvas3d::mouse_event(const Vec2& size, const MouseEvent& event) {
  // TODO
}

}; // namespace datagui
