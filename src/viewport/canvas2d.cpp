#include "datagui/viewport/canvas2d.hpp"
#include "datagui/visual/color_map.hpp"

namespace dgui {

void Canvas2d::rect(
    const Vec2& position,
    float angle,
    const Vec2& size,
    const Color& color,
    float border_width,
    const Color& border_color) {
  shape_shader
      .queue_rect(position, angle, size, color, border_width, border_color);
}

void Canvas2d::circle(
    const Vec2& position,
    float radius,
    const Color& color,
    float border_width,
    const Color& border_color) {
  shape_shader
      .queue_circle(position, radius, color, border_width, border_color);
}

void Canvas2d::ellipse(
    const Vec2& position,
    float angle,
    const Vec2& radii,
    const Color& color,
    float border_width,
    const Color& border_color) {
  shape_shader
      .queue_ellipse(position, angle, radii, color, border_width, border_color);
}

void Canvas2d::line(
    const Vec2& a,
    const Vec2& b,
    float width,
    const Color& color) {
  shape_shader.queue_line(a, b, width, color);
}

void Canvas2d::capsule(
    const Vec2& a,
    const Vec2& b,
    float radius,
    const Color& color,
    float border_width,
    const Color& border_color) {
  shape_shader.queue_capsule(a, b, radius, color, border_width, border_color);
}

void Canvas2d::text(
    const std::string& text,
    const Vec2& origin,
    float angle,
    int font_size,
    Font font,
    Color text_color,
    Length width) {
  if (!prev_viewport_) {
    return;
  }
  float text_scale = camera.size.x / prev_viewport_->size().x;

  static constexpr int max_font_size = 80;
  static constexpr int min_font_size = 12;
  // Avoid generating too many fonts, choose font sizes at multiples of 4
  // and adjust zoom appropriately
  static constexpr int font_size_modulo = 4;

  const int scaled_font_size = static_cast<int>(font_size * zoom);
  const int font_size_used = std::clamp(
      scaled_font_size - (scaled_font_size % font_size_modulo),
      min_font_size,
      max_font_size);
  const float zoom_used = (font_size * zoom) / font_size_used;

  text_shader.queue_text(
      origin,
      angle,
      Vec2::uniform(text_scale * zoom_used),
      text,
      font,
      font_size_used,
      text_color,
      width);
}

void Canvas2d::heatmap(
    const Vec2& lower,
    const Vec2& upper,
    const std::function<float(float x, float y)>& function,
    float min_value,
    float max_value,
    std::size_t width,
    std::size_t height) {
  struct Pixel {
    std::uint8_t r, g, b, a;
  };
  std::vector<Pixel> pixels(width * height);
  for (std::size_t i = 0; i < height; i++) {
    for (std::size_t j = 0; j < width; j++) {
      Vec2 normalized =
          Vec2(float(j) + 0.5, height - (float(i) + 0.5)) / Vec2(width, height);
      Vec2 coords = lower + (upper - lower) * normalized;
      float value = function(coords.x, coords.y);

      Vec3 color;
      if (min_value < 0 && max_value > 0) {
        if (value >= 0) {
          Vec3 min = {0, 1, 1};
          Vec3 max = {0, 0, 1};
          float s = std::min(value, max_value) / max_value;
          color = (1 - s) * min + s * max;
        } else {
          Vec3 min = {1, 1, 0};
          Vec3 max = {1, 0, 0};
          float s = std::max(value, min_value) / min_value;
          color = (1 - s) * min + s * max;
        }
        float s = (value - min_value) / (max_value - min_value);
        Vec3 color = color_map_viridis(s);
        s = std::clamp(s, 0.f, 1.f);
      } else {
        float s = (value - min_value) / (max_value - min_value);
        s = std::clamp(s, 0.f, 1.f);
        color = color_map_viridis(s);
      }

      auto& pixel = pixels[i * width + j];
      pixel.r = color.x * 255;
      pixel.g = color.y * 255;
      pixel.b = color.z * 255;
      pixel.a = 255;
    }
  }

  Image image;
  image.load(width, height, pixels.data());
  image_shader.queue_image(image, lower, 0, upper - lower);
}

std::optional<MouseEvent> Canvas2d::mouse_event() {
  if (mouse_event_) {
    return std::move(mouse_event_);
  }
  return std::nullopt;
}

void Canvas2d::init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontManager>& fm) {
  bg_shader.init();
  shape_shader.init();
  text_shader.init(fm);
  image_shader.init();
}

void Canvas2d::begin() {
  shape_shader.clear();
  text_shader.clear();
  image_shader.clear();
  bg_color_ = Color::Gray(0.95);
  default_position_ = Vec2();
  default_view_width_ = 1;
  aspect_ratio_ = 1;
}

void Canvas2d::draw(const Box2& viewport, const Box2& mask) {
  prev_viewport_ = viewport;
  camera.size.x = default_view_width_ / zoom;
  camera.size.y = camera.size.x / aspect_ratio_;

  // Now modify the camera so it fits the masked area instead
  Box2 masked_area = intersection(viewport, mask);
  Box2 normalized_area;
  normalized_area.lower =
      (masked_area.lower - viewport.lower) / viewport.size();
  normalized_area.upper =
      normalized_area.lower + masked_area.size() / viewport.size();

  Camera2d cropped_camera;
  cropped_camera.position =
      camera.position +
      camera.size * (normalized_area.center() - Vec2::uniform(0.5));
  cropped_camera.size = camera.size * normalized_area.size();

  bg_shader
      .queue_rect(cropped_camera.position, 0, cropped_camera.size, bg_color_);
  bg_shader.draw(masked_area, cropped_camera);
  bg_shader.clear();

  image_shader.draw(masked_area, cropped_camera);
  shape_shader.draw(masked_area, cropped_camera);
  text_shader.draw(masked_area, cropped_camera);
}

void Canvas2d::mouse_event(const MouseEvent& event) {
  if (event.button != MouseButton::Middle) {
    MouseEvent remapped = event;
    remapped.press_position = camera.from_camera(event.press_position);
    remapped.position = camera.from_camera(event.position);
    mouse_event_ = remapped;
    return;
  }
  if (event.action == MouseAction::Press) {
    if (event.mod.ctrl) {
      camera.position = Vec2();
      zoom = 1;
    }
    click_camera = camera;
    return;
  }

  camera.position = click_camera.position +
                    click_camera.from_camera(event.press_position) -
                    click_camera.from_camera(event.position);
}

bool Canvas2d::scroll_event(const ScrollEvent& event) {
  float change_factor = std::exp(-event.amount / 250);
  camera.size /= change_factor;
  zoom *= change_factor;
  camera.position +=
      (event.position - Vec2::uniform(0.5)) * (change_factor - 1) * camera.size;
  return true;
}

}; // namespace dgui
