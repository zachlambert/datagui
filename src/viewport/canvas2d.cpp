#include "datagui/viewport/canvas2d.hpp"
#include "datagui/visual/color_map.hpp"

namespace datagui {

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
    const Vec2& origin,
    float angle,
    const std::string& text,
    Font font,
    int font_size,
    Color text_color,
    Length width) {
  text_shader
      .queue_text(origin, angle, text, font, font_size, text_color, width);
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
    ;
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

void Canvas2d::begin() {
  shape_shader.clear();
  text_shader.clear();
  image_shader.clear();
  bg_color_ = Color::Gray(0.95);
  scale_ = 1;
}

void Canvas2d::end() {
  redraw();
}

void Canvas2d::impl_init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontManager>& fm) {
  shape_shader.init();
  text_shader.init(fm);
  image_shader.init();
}

void Canvas2d::redraw() {
  bind_framebuffer(bg_color_);
  camera.size = viewport().size();
  double original_zoom = camera.zoom;
  camera.zoom *= (viewport().size().x / scale_);
  image_shader.draw(viewport(), camera);
  shape_shader.draw(viewport(), camera);
  text_shader.draw(viewport(), camera);
  camera.zoom = original_zoom;
  unbind_framebuffer();
}

void Canvas2d::mouse_event(const Vec2& size, const MouseEvent& event) {
  if (event.button != MouseButton::Left) {
    return;
  }
  if (event.action == MouseAction::Press) {
    if (event.is_double_click) {
      camera.position = Vec2();
      camera.zoom = 1;
    }
    click_mouse_pos = event.position;
    click_camera_pos = camera.position;
    return;
  }

  double full_scale = camera.zoom * (viewport().size().x / scale_);
  camera.position =
      click_camera_pos - (event.position - click_mouse_pos) / full_scale;
  redraw();
}

bool Canvas2d::scroll_event(const Vec2& size, const ScrollEvent& event) {
  camera.zoom *= std::exp(-event.amount / 1000);
  redraw();
  return true;
}

}; // namespace datagui
