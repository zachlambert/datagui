#include "datagui/system/color_picker.hpp"

namespace datagui {

ColorPickerSystem::ColorPickerSystem(
    std::shared_ptr<FontManager> fm,
    std::shared_ptr<Theme> theme) :
    fm(fm), theme(theme) {
  //
}

void ColorPickerSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& color_picker = element.color_picker();

  state.fixed_size = Vec2::uniform(theme->color_picker_icon_size);
  state.dynamic_size = Vec2();

  Vec2 float_size;
  float_size.x = 2 * theme->color_picker_hue_wheel_radius +
                 4 * theme->color_picker_padding +
                 theme->color_picker_value_scale_width,
  float_size.y = 2 * theme->color_picker_hue_wheel_radius + //
                 2 * theme->color_picker_padding;

  // TODO: Use param for border size

  state.floating = color_picker.open;
  state.floating_type = FloatingTypeRelative(
      Vec2(0, theme->color_picker_icon_size - 2),
      float_size);
}

void ColorPickerSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  const auto& color_picker = element.color_picker();

  // TODO: Store wheel and value boxes
}

void ColorPickerSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& color_picker = element.color_picker();

  renderer
      .queue_box(state.box(), color_picker.value, 2, theme->input_color_border);
  if (!state.floating) {
    return;
  }

  Color bg_color = color_picker.value;
  bg_color.a = 0.5;
  renderer.queue_box(state.float_box, bg_color, 2, theme->layout_border_color);

  float value = color_picker.value.value();
  struct Pixel {
    std::uint8_t r, g, b, a;
  };
  {
    const std::size_t n = 200;
    float half_width = float(n) / 2;
    std::vector<Pixel> pixels(n * n);
    for (std::size_t i = 0; i < n; i++) {
      for (std::size_t j = 0; j < n; j++) {
        Pixel& pixel = pixels[i * n + j];
        float x = (float(j) - half_width) / half_width;
        float y = (float(i) - half_width) / half_width;
        float saturation = std::hypot(x, y);
        if (saturation > 1) {
          pixel.a = 0;
          continue;
        }
        // Aliasing
        pixel.a = 255 * std::min((1 - saturation) / 0.02f, 1.0f);
        float angle = std::atan2(-y, -x);
        if (angle < 0) {
          angle += 2 * M_PI;
        }
        float hue = 180 * angle / M_PI;
        Color color = Color::Hsl(hue, saturation, value);
        pixel.r = color.r * 255;
        pixel.g = color.g * 255;
        pixel.b = color.b * 255;
      }
    }
    Vec2 pos =
        state.float_box.lower + Vec2::uniform(theme->color_picker_padding);
    Vec2 size = Vec2::uniform(theme->color_picker_hue_wheel_radius * 2);
    renderer.queue_image(Box2(pos, pos + size), n, n, pixels.data());
  }
  {
    const std::size_t h = 100;
    const std::size_t w = 100 * theme->color_picker_value_scale_width /
                          (2 * theme->color_picker_hue_wheel_radius);

    std::vector<Pixel> pixels(w * h);
    for (std::size_t i = 0; i < h; i++) {
      std::uint8_t value = (float(i) / h) * 255;
      for (std::size_t j = 0; j < w; j++) {
        Pixel& pixel = pixels[i * w + j];
        pixel.r = value;
        pixel.g = value;
        pixel.b = value;
        pixel.a = 255;
      }
    }
    Vec2 pos = state.float_box.lower_right();
    pos.x -=
        theme->color_picker_value_scale_width + theme->color_picker_padding;
    pos.y += theme->color_picker_padding;
    Vec2 size = Vec2(
        theme->color_picker_value_scale_width,
        theme->color_picker_hue_wheel_radius * 2);
    renderer.queue_image(Box2(pos, pos + size), w, h, pixels.data());
  }
}

bool ColorPickerSystem::mouse_event(
    ElementPtr element,
    const MouseEvent& event) {
  const auto& state = element.state();
  auto& color_picker = element.color_picker();

  if (event.action == MouseAction::Release) {
    color_picker.wheel_held = false;
    color_picker.scale_held = false;
    if (!color_picker.open) {
      color_picker.open = true;
      return false;
    } else {
      if (color_picker.modified) {
        color_picker.modified = false;
        if (color_picker.callback) {
          color_picker.callback(color_picker.value);
          return false;
        }
        return true;
      }
      return false;
    }
  }
  if (!color_picker.open) {
    return false;
  }

  Vec2 hue_wheel_origin =
      state.float_box.lower +
      Vec2::uniform(
          theme->color_picker_padding + theme->color_picker_hue_wheel_radius);
  Vec2 hue_wheel_offset = event.position - hue_wheel_origin;

  if (event.action == MouseAction::Press && !color_picker.wheel_held) {
    if (hue_wheel_offset.length() < theme->color_picker_hue_wheel_radius) {
      color_picker.wheel_held = true;
    }
  }
  if (color_picker.wheel_held) {
    float angle = std::atan2(hue_wheel_offset.y, -hue_wheel_offset.x);
    if (angle < 0) {
      angle += 2 * M_PI;
    }
    float hue = 180 * angle / M_PI;
    float saturation =
        hue_wheel_offset.length() / theme->color_picker_hue_wheel_radius;
    saturation = std::clamp(saturation, 0.f, 1.f);

    Color new_color = Color::Hsl(hue, saturation, color_picker.value.value());
    color_picker.value = new_color;
    color_picker.modified = true;
  }

  Vec2 value_box_origin;
  value_box_origin.x = state.float_box.upper.x - theme->color_picker_padding -
                       theme->color_picker_value_scale_width;
  value_box_origin.y = state.float_box.lower.y + theme->color_picker_padding;

  Vec2 value_box_size = Vec2(
      theme->color_picker_value_scale_width,
      2 * theme->color_picker_hue_wheel_radius);

  Box2 value_box(value_box_origin, value_box_origin + value_box_size);

  if (event.action == MouseAction::Press &&
      value_box.contains(event.position)) {
    color_picker.scale_held = true;
  }
  if (color_picker.scale_held) {
    float value = (value_box.upper.y - event.position.y) / value_box_size.y;
    value = std::clamp(value, 0.f, 1.f);

    Color new_color = Color::Hsl(
        color_picker.value.hue(),
        color_picker.value.saturation(),
        value);
    color_picker.value = new_color;
  }

  return false;
}

bool ColorPickerSystem::focus_leave(ElementPtr element, bool success) {
  auto& color_picker = element.color_picker();
  color_picker.open = false;
  return false;
}

} // namespace datagui
