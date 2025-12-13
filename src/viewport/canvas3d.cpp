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

void Canvas3d::cylinder(
    const Vec3& base_position,
    const Vec3& direction,
    float radius,
    float length,
    const Color& color) {
  shape_shader.queue_cylinder(base_position, direction, radius, length, color);
}

void Canvas3d::grid(std::size_t size, float width) {
  shape_shader.queue_box(
      Vec3(0, 0, -0.05),
      Rot3(),
      Vec3(width, width, 0.1),
      Color::Gray(0.8));
}

void Canvas3d::begin() {
  shape_shader.clear();
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
  Vec3 direction;
  direction.x = (event.position.x - size.x / 2) / (size.x / 2);
  direction.y = (event.position.y - size.y / 2) / (size.y / 2);
  direction.z = -1;
  // Not normalized
  Vec3 direction_world = camera.rotation().mat() * direction;

  if (event.action == MouseAction::Press) {
    float distance = camera.position.z / (-direction_world.z);
    click_point = camera.position + distance * direction_world;
    click_distance_z = distance * std::abs(direction.z);
    click_point_direction = direction;
    click_camera_direction = camera.direction;
    return;
  }
  if (event.action != MouseAction::Hold) {
    return;
  }

  if (event.button == MouseButton::Left) {
    float pitch = std::asin(-click_camera_direction.z) +
                  std::atan(direction.y) - std::atan(click_point_direction.y);
    pitch = std::clamp(pitch, -0.4f * M_PIf, 0.4f * M_PIf);
    float yaw =
        std::atan2(click_camera_direction.y, click_camera_direction.x) +
        std::atan2(std::sin(pitch) + std::cos(pitch), -direction.x) -
        std::atan2(std::sin(pitch) + std::cos(pitch), -click_point_direction.x);

    camera.direction = {
        std::cos(yaw) * std::cos(pitch),
        std::sin(yaw) * std::cos(pitch),
        -std::sin(pitch),
    };
  } else if (event.button == MouseButton::Middle) {
    Vec3 direction_world = camera.rotation().mat() * direction;
    camera.position = click_point - direction_world * click_distance_z;
  }

  bind_framebuffer();
  shape_shader.draw(framebuffer_size(), camera);
  unbind_framebuffer();
}

bool Canvas3d::scroll_event(const Vec2& size, const ScrollEvent& event) {
  Vec3 direction;
  direction.x = (event.position.x - size.x / 2) / (size.x / 2);
  direction.y = (event.position.y - size.y / 2) / (size.y / 2);
  direction.z = -1;
  // Not normalized
  Vec3 direction_world = camera.rotation().mat() * direction;

  float distance = camera.position.z / (-direction_world.z);
  float new_distance = std::exp(-event.amount / 1000) * distance;
  camera.position += direction_world * (new_distance - distance);

  bind_framebuffer();
  shape_shader.draw(framebuffer_size(), camera);
  unbind_framebuffer();
  return true;
}

}; // namespace datagui
