#include "datagui/viewport/canvas3d.hpp"

namespace datagui {

Canvas3d::Canvas3d() {
  reset_camera();
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
  float line_width = 0.02;
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
  bg_color_ = Color::Gray(0.95);
}

void Canvas3d::end() {
  redraw();
}

void Canvas3d::redraw() {
  bind_framebuffer(bg_color_);
  camera.fov.y = camera.fov.x * viewport().ratio_yx();
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

void Canvas3d::mouse_event(const MouseEvent& event) {
  if (event.button == MouseButton::Left && event.is_double_click) {
    reset_camera();
    redraw();
  }
  if (event.button != MouseButton::Middle) {
#if 0
    if (click_callback_) {
      click_callback_(event);
    }
#endif
    return;
  }
  if (event.action == MouseAction::Press) {
    click_camera = camera;
    return;
  }

  if (event.mod.shift) {
    Vec3 delta_cs = 2 * (click_camera.ray_camera(event.position) -
                         click_camera.ray_camera(event.press_position));
    float yaw_change = std::atan2(delta_cs.x, 1);
    float pitch_change = std::atan2(delta_cs.y, 1);
    float click_yaw =
        std::atan2(click_camera.direction.y, click_camera.direction.x);
    float click_pitch = std::atan2(
        -click_camera.direction.z,
        std::hypot(click_camera.direction.x, click_camera.direction.y));
    float yaw = click_yaw + yaw_change;
    float pitch = click_pitch + pitch_change;
    camera.direction = {
        std::cos(yaw) * std::cos(pitch),
        std::sin(yaw) * std::cos(pitch),
        -std::sin(pitch),
    };
  } else {
    float distance = click_camera.position.z /
                     click_camera.direction_world(event.press_position).z;
    Vec3 delta_cs = 2 *
                    (click_camera.ray_camera(event.position) -
                     click_camera.ray_camera(event.press_position)) *
                    distance;
    camera.position =
        click_camera.position + click_camera.rotation() * delta_cs;
  }

  redraw();
}

bool Canvas3d::scroll_event(const ScrollEvent& event) {
  Vec3 direction = camera.direction_world(event.position);
  float distance = camera.position.z / (-direction.z);
  float new_distance = std::exp(-event.amount / 250) * distance;
  camera.position += direction * (new_distance - distance);
  redraw();
  return true;
}

void Canvas3d::reset_camera() {
  camera.direction = Rot3(Euler(0, M_PI / 6, M_PI / 4)).mat() * Vec3(1, 0, 0);
  camera.position.x = -7;
  camera.position.y = -7;
  camera.position.z = 5;
  camera.fov.x = M_PI * 70 / 180;
  // Set fov.y during redraw since window size may change
  camera.clipping_min = 0.001;
  camera.clipping_max = 1000;
}

}; // namespace datagui
