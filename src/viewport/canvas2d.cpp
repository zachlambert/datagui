#include "datagui/viewport/canvas2d.hpp"
#include "datagui/render/lookup/color_map.hpp"

namespace dgui {

void Canvas2d::rect(
    const Vec2& position,
    float angle,
    const Vec2& size,
    const Color& color,
    float border_width,
    const Color& border_color) {
  scene->draw_rect(position, angle, size, color, border_width, border_color);
}

void Canvas2d::circle(
    const Vec2& position,
    float radius,
    const Color& color,
    float border_width,
    const Color& border_color) {
  scene->draw_circle(position, radius, color, border_width, border_color);
}

void Canvas2d::ellipse(
    const Vec2& position,
    float angle,
    const Vec2& radii,
    const Color& color,
    float border_width,
    const Color& border_color) {
  scene
      ->draw_ellipse(position, angle, radii, color, border_width, border_color);
}

void Canvas2d::line(
    const Vec2& a,
    const Vec2& b,
    float width,
    const Color& color) {
  scene->draw_line(a, b, width, color, false);
}

void Canvas2d::capsule(
    const Vec2& a,
    const Vec2& b,
    float radius,
    const Color& color,
    float border_width,
    const Color& border_color) {
  scene->draw_capsule(a, b, radius, color, border_width, border_color);
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

  scene->draw_text(
      font_registry->get_font(font, font_size_used),
      origin,
      angle,
      Vec2::uniform(text_scale * zoom_used),
      text_color,
      width,
      text);
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
  scene->draw_image(image, lower, 0, upper - lower);
}

std::optional<MouseEvent> Canvas2d::mouse_event() {
  if (mouse_event_) {
    return std::move(mouse_event_);
  }
  return std::nullopt;
}

void Canvas2d::init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontRegistry>& font_registry) {
  this->font_registry = font_registry;
}

void Canvas2d::begin() {
  scene = std::make_shared<Scene2d>();
  scene->bg_color = Color::Gray(0.95);
  default_position_ = Vec2();
  default_view_width_ = 1;
}

void Canvas2d::draw(const Box2& viewport, DrawList& dl) {
  prev_viewport_ = viewport;
  camera.size.x = default_view_width_ / zoom;
  camera.size.y = viewport.ratio_yx() * camera.size.x;
  dl.draw_scene_2d(viewport, camera, scene);
}

void Canvas2d::mouse_event(const MouseEvent& event) {
  if (event.button == MouseButton::Right) {
    if (event.action == MouseAction::Press) {
      if (event.mod.ctrl) {
        camera.position = Vec2();
        zoom = 1;
      }
      click_camera = camera;
    } else {
      camera.position = click_camera.position +
                        click_camera.from_camera(event.press_position) -
                        click_camera.from_camera(event.position);
    }
  }

  MouseEvent remapped = event;
  remapped.press_position = camera.from_camera(event.press_position);
  remapped.position = camera.from_camera(event.position);
  mouse_event_ = remapped;
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
