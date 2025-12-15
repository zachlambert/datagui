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

void Canvas3d::sphere(const Vec3& position, float radius, const Color& color) {
  shape_shader.queue_sphere(position, radius, color);
}

void Canvas3d::cone(
    const Vec3& base_position,
    const Vec3& direction,
    float radius,
    float length,
    const Color& color) {
  shape_shader.queue_cone(base_position, direction, radius, length, color);
}

void Canvas3d::capsule(
    const Vec3& start,
    const Vec3& end,
    float radius,
    const Color& color) {
  shape_shader.queue_capsule(start, end, radius, color);
}

void Canvas3d::arrow(
    const Vec3& start,
    const Vec3& end,
    float radius,
    const Color& color,
    float head_length_scale,
    float head_radius_scale) {
  shape_shader.queue_arrow(
      start,
      end,
      radius,
      color,
      head_length_scale,
      head_radius_scale);
}

void Canvas3d::plane(
    const Vec3& position,
    const Rot3& orientation,
    const Vec2& scale,
    const Color& color) {
  shape_shader.queue_plane(position, orientation, scale, color);
}

void Canvas3d::axes(
    const Vec3& position,
    const Rot3& orientation,
    float scale,
    float line_radius,
    float head_length_scale,
    float head_radius_scale) {
  shape_shader.queue_arrow(
      position,
      position + orientation.mat() * (scale * Vec3::unit_x()),
      line_radius,
      Color::Red(),
      head_length_scale,
      head_radius_scale);

  shape_shader.queue_arrow(
      position,
      position + orientation.mat() * (scale * Vec3::unit_y()),
      line_radius,
      Color::Green(),
      head_length_scale,
      head_radius_scale);

  shape_shader.queue_arrow(
      position,
      position + orientation.mat() * (scale * Vec3::unit_z()),
      line_radius,
      Color::Blue(),
      head_length_scale,
      head_radius_scale);

  shape_shader.queue_sphere(position, line_radius, Color::Gray(0.5));
}

void Canvas3d::grid(std::size_t size, float width) {
  Color color = Color::Gray(0.8);
  float line_width = 0.05;
  for (std::size_t i = 0; i <= size; i++) {
    float x = -width / 2 + i * width / size;
    shape_shader.queue_plane(
        Vec3(x, 0, 0),
        Rot3(),
        Vec2(line_width, width + line_width),
        color);
  }
  for (std::size_t i = 0; i <= size; i++) {
    float y = -width / 2 + i * width / size;
    shape_shader.queue_plane(
        Vec3(0, y, 0),
        Rot3(),
        Vec2(width + line_width, line_width),
        color);
  }
}

void Canvas3d::mesh(
    const Mesh& mesh,
    const Vec3& position,
    const Rot3& orientation,
    const Color& color) {
  mesh_shader.queue_mesh(mesh, position, orientation, color);
}

void Canvas3d::uv_mesh(
    const UvMesh& uv_mesh,
    const Vec3& position,
    const Rot3& orientation,
    float opacity) {
  uv_mesh_shader.queue_mesh(uv_mesh, position, orientation, opacity);
}

void Canvas3d::point_cloud(
    const PointCloud& point_cloud,
    const Vec3& position,
    const Rot3& orientation,
    float point_size) {
  point_cloud_shader
      .queue_point_cloud(point_cloud, position, orientation, point_size);
}

void Canvas3d::begin() {
  shape_shader.clear();
  mesh_shader.clear();
  uv_mesh_shader.clear();
  point_cloud_shader.clear();
}

void Canvas3d::end() {
  redraw();
}

void Canvas3d::redraw() {
  bind_framebuffer();
  shape_shader.draw(viewport(), camera);
  mesh_shader.draw(viewport(), camera);
  uv_mesh_shader.draw(viewport(), camera);
  point_cloud_shader.draw(viewport(), camera);
  unbind_framebuffer();
}

void Canvas3d::impl_init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontManager>& fm) {
  shape_shader.init();
  mesh_shader.init();
  uv_mesh_shader.init();
  point_cloud_shader.init();
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

  redraw();
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

  redraw();
  return true;
}

}; // namespace datagui
