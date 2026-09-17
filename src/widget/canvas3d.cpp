#include "datagui/widget/canvas3d.hpp"

namespace dgui {

Canvas3d::Canvas3d() {
  reset_camera();
}

void Canvas3d::box(
    const Vec3& position,
    const Rot3& orientation,
    const Vec3& size,
    const Color& color) {
  scene->draw_box(position, orientation, size, color);
}

void Canvas3d::cylinder(
    const Vec3& base_position,
    const Vec3& direction,
    float radius,
    float length,
    const Color& color) {
  scene->draw_cylinder(base_position, direction, radius, length, color);
}

void Canvas3d::sphere(const Vec3& position, float radius, const Color& color) {
  scene->draw_sphere(position, radius, color);
}

void Canvas3d::cone(
    const Vec3& base_position,
    const Vec3& direction,
    float radius,
    float length,
    const Color& color) {
  scene->draw_cone(base_position, direction, radius, length, color);
}

void Canvas3d::capsule(
    const Vec3& start,
    const Vec3& end,
    float radius,
    const Color& color) {
  scene->draw_capsule(start, end, radius, color);
}

void Canvas3d::arrow(
    const Vec3& start,
    const Vec3& end,
    float radius,
    const Color& color,
    float head_length_scale,
    float head_radius_scale) {
  scene->draw_arrow(
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
  scene->draw_plane(position, orientation, scale, color);
}

void Canvas3d::axes(
    const Vec3& position,
    const Rot3& orientation,
    float scale,
    float line_radius,
    float head_length_scale,
    float head_radius_scale) {
  scene->draw_arrow(
      position,
      position + orientation.mat() * (scale * Vec3::unit_x()),
      line_radius,
      Color::Red(),
      head_length_scale,
      head_radius_scale);

  scene->draw_arrow(
      position,
      position + orientation.mat() * (scale * Vec3::unit_y()),
      line_radius,
      Color::Green(),
      head_length_scale,
      head_radius_scale);

  scene->draw_arrow(
      position,
      position + orientation.mat() * (scale * Vec3::unit_z()),
      line_radius,
      Color::Blue(),
      head_length_scale,
      head_radius_scale);

  scene->draw_sphere(position, line_radius, Color::Gray(0.5));
}

void Canvas3d::grid(std::size_t size, float width) {
  Color color = Color::Gray(0.8);
  float line_width = 0.02;
  for (std::size_t i = 0; i <= size; i++) {
    float x = -width / 2 + i * width / size;
    scene->draw_plane(
        Vec3(x, 0, 0),
        Rot3(),
        Vec2(line_width, width + line_width),
        color);
  }
  for (std::size_t i = 0; i <= size; i++) {
    float y = -width / 2 + i * width / size;
    scene->draw_plane(
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
  scene->draw_mesh(mesh, position, orientation, Vec3::ones(), color);
}

void Canvas3d::uv_mesh(
    const UvMesh& uv_mesh,
    const Vec3& position,
    const Rot3& orientation,
    float opacity) {
  // TODO: Scene3d has no textured mesh support, and there is no uv mesh
  // program in the registry, so this can't be queued yet
}

void Canvas3d::point_cloud(
    const PointCloud& point_cloud,
    const Vec3& position,
    const Rot3& orientation,
    float point_size) {
  scene->draw_point_cloud(
      point_cloud,
      position,
      orientation,
      Vec3::ones(),
      point_size);
}

void Canvas3d::begin() {
  if (!scene) {
    scene = std::make_shared<Scene3d>();
  }
  scene->clear();
  scene->bg_color = Color::Gray(0.95);
  aspect_ratio_ = 1;
  click_callback_ = {};
}

void Canvas3d::init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontRegistry>& font_registry) {}

void Canvas3d::set_dependent_state(const Box2& box) {
  camera.fov.y =
      2.f * std::atan(std::tan(0.5f * camera.fov.x) * box.ratio_yx());
}

void Canvas3d::render(const Box2& box, DrawList& dl) const {
  dl.draw_box(box, Color::Black(), border_width_);
  dl.draw_scene_3d(box.from_shrink(border_width_), camera, scene);
}

void Canvas3d::mouse_event(const Box2& box, const MouseEvent& event) {
  const Box2 canvas = box.from_shrink(border_width_);
  const Vec2 position_coords =
      remap_flip_y(event.position, canvas, Box2::unit_box());
  const Vec2 press_position_coords =
      remap_flip_y(event.press_position, canvas, Box2::unit_box());

  if (event.button == MouseButton::Right) {
    if (event.action == MouseAction::Press) {
      if (event.is_double_click) {
        reset_camera();
      }
      click_camera = camera;
    }
    if (event.mod.shift) {
      Vec3 delta_cs =
          (click_camera.ray_camera(position_coords) -
           click_camera.ray_camera(press_position_coords));
      float yaw_change = std::atan2(delta_cs.x, 1);
      float pitch_change = std::atan2(delta_cs.y, 1);
      float click_yaw =
          std::atan2(click_camera.direction.y, click_camera.direction.x);
      float click_pitch = std::atan2(
          -click_camera.direction.z,
          std::hypot(click_camera.direction.x, click_camera.direction.y));

      float yaw = click_yaw + yaw_change;
      float pitch = click_pitch + pitch_change;
      pitch = std::clamp(pitch, -M_PIf * 0.48f, M_PIf * 0.48f);

      camera.direction = {
          std::cos(yaw) * std::cos(pitch),
          std::sin(yaw) * std::cos(pitch),
          -std::sin(pitch),
      };
    } else {
      float distance = click_camera.position.z /
                       click_camera.direction_world(press_position_coords).z;
      Vec3 delta_cs = (click_camera.ray_camera(position_coords) -
                       click_camera.ray_camera(press_position_coords)) *
                      distance;
      camera.position =
          click_camera.position + click_camera.rotation() * delta_cs;
    }
  }

  if (click_callback_) {
    Vec3 press_ray = camera.ray_camera(press_position_coords);
    Vec3 ray = camera.ray_camera(position_coords);
    MouseEvent remapped = event;
    remapped.press_position = Vec2(press_ray.x, press_ray.y);
    remapped.position = Vec2(ray.x, ray.y);
    click_callback_(remapped);
  }
}

bool Canvas3d::scroll_event(const Box2& box, const ScrollEvent& event) {
  const Vec2 position_coords = remap_flip_y(
      event.position,
      box.from_shrink(border_width_),
      Box2::unit_box());

  float distance = -event.amount / 20.f;
  if (event.mod.shift) {
    distance /= 10;
  }
  if (event.mod.ctrl) {
    distance *= 10;
  }
  camera.position += camera.direction_world(position_coords) * distance;
  return true;
}

void Canvas3d::reset_camera() {
  camera.direction = Rot3(Euler(0, M_PI / 6, M_PI / 4)).mat() * Vec3(1, 0, 0);
  camera.position.x = -7;
  camera.position.y = -7;
  camera.position.z = 5;
  camera.fov.x = M_PI * 70 / 180;
  camera.clipping_min = 0.001;
  camera.clipping_max = 1000;
}

}; // namespace dgui
