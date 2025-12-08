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
  auto& color_picker = element.color_picker();

  state.fixed_size = Vec2::uniform(theme->color_picker_icon_size);
  state.dynamic_size = Vec2();

  Vec2 float_offset(
      0,
      theme->color_picker_icon_size - theme->input_border_width);

  const float r = theme->color_picker_hue_wheel_radius;
  const float p = theme->color_picker_padding;
  const float w = theme->color_picker_value_scale_width;
  Vec2 float_size(3 * p + 2 * r + w, 2 * p + 2 * r);

  state.floating = color_picker.open;
  state.floating_type = FloatingTypeRelative(float_offset, float_size);
  state.float_only = false;
}

void ColorPickerSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& color_picker = element.color_picker();

  const float r = theme->color_picker_hue_wheel_radius;
  const float p = theme->color_picker_padding;
  const float w = theme->color_picker_value_scale_width;

  {
    Vec2 origin = state.float_box.lower + Vec2::uniform(p);
    Vec2 size = Vec2::uniform(2 * r);
    color_picker.hue_wheel_box = Box2(origin, origin + size);
  }

  {
    Vec2 origin = state.float_box.lower + Vec2(2 * p + 2 * r, p);
    Vec2 size = Vec2(w, 2 * r);
    color_picker.lightness_box = Box2(origin, origin + size);
  }
}

void ColorPickerSystem::render(ConstElementPtr element, GuiRenderer& renderer) {
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

  float lightness = color_picker.value.lightness();
  struct Pixel {
    std::uint8_t r, g, b, a;
  };
  {
    const std::size_t n = 200;
    float half_width = float(n) / 2;

    float angle = M_PI + M_PI * color_picker.value.hue() / 180;
    float saturation = color_picker.value.saturation();
    std::uint8_t marker_value = (lightness < 0.5 ? 255 : 0);
    float marker_x = std::cos(angle) * saturation;
    float marker_y = std::sin(angle) * saturation;

    std::vector<Pixel> pixels(n * n);
    for (std::size_t i = 0; i < n; i++) {
      for (std::size_t j = 0; j < n; j++) {
        Pixel& pixel = pixels[i * n + j];
        float x = (float(j) - half_width) / half_width;
        float y = (float(i) - half_width) / half_width;

        if (std::hypot(x - marker_x, y - marker_y) < 10.f / n) {
          pixel.r = marker_value;
          pixel.g = marker_value;
          pixel.b = marker_value;
          pixel.a = 255;
          continue;
        }

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
        Color color = Color::Hsl(hue, saturation, lightness);
        pixel.r = color.r * 255;
        pixel.g = color.g * 255;
        pixel.b = color.b * 255;
      }
    }
    renderer.queue_image(color_picker.hue_wheel_box, n, n, pixels.data());
  }
  {
    const std::size_t h = 100;
    const std::size_t w = 100 * color_picker.lightness_box.size().x /
                          color_picker.lightness_box.size().y;

    std::uint8_t marker_value = lightness > 0.5 ? 0 : 255;
    std::size_t marker_lower = h * std::max(lightness - 0.02f, 0.f);
    std::size_t marker_upper = h * std::min(lightness + 0.02f, 1.f);

    std::vector<Pixel> pixels(w * h);
    for (std::size_t i = 0; i < h; i++) {
      std::uint8_t value = 255 * (float(i) / h);
      if (i >= marker_lower && i < marker_upper) {
        value = marker_value;
      }

      for (std::size_t j = 0; j < w; j++) {
        Pixel& pixel = pixels[i * w + j];
        pixel.r = value;
        pixel.g = value;
        pixel.b = value;
        pixel.a = 255;
      }
    }
    renderer.queue_image(color_picker.lightness_box, w, h, pixels.data());
  }
}

void ColorPickerSystem::mouse_event(
    ElementPtr element,
    const MouseEvent& event) {
  const auto& state = element.state();
  auto& color_picker = element.color_picker();

  if (event.action == MouseAction::Release) {
    color_picker.wheel_held = false;
    color_picker.scale_held = false;
    if (!color_picker.open) {
      color_picker.open = true;
      return;
    }
    if (color_picker.modified) {
      color_picker.modified = false;
      if (color_picker.callback) {
        color_picker.callback(color_picker.value);
      } else {
        element.set_dirty();
      }
    }
    return;
  }
  if (!color_picker.open) {
    return;
  }

  Vec2 hue_wheel_offset = event.position - color_picker.hue_wheel_box.center();

  if (event.action == MouseAction::Press && !color_picker.wheel_held) {
    if (hue_wheel_offset.length() <= theme->color_picker_hue_wheel_radius) {
      color_picker.wheel_held = true;
    }
  }
  if (color_picker.wheel_held) {
    float angle = std::atan2(-hue_wheel_offset.y, -hue_wheel_offset.x);
    if (angle < 0) {
      angle += 2 * M_PI;
    }
    float hue = 180 * angle / M_PI;
    float saturation =
        hue_wheel_offset.length() / theme->color_picker_hue_wheel_radius;
    saturation = std::clamp(saturation, 0.f, 1.f);

    Color new_color =
        Color::Hsl(hue, saturation, color_picker.value.lightness());
    color_picker.value = new_color;
    if (color_picker.always) {
      color_picker.callback(new_color);
    } else {
      color_picker.modified = true;
    }
  }

  if (event.action == MouseAction::Press &&
      color_picker.lightness_box.contains(event.position)) {
    color_picker.scale_held = true;
  }
  if (color_picker.scale_held) {
    float lightness = (event.position.y - color_picker.lightness_box.lower.y) /
                      color_picker.lightness_box.size().y;
    lightness = std::clamp(lightness, 0.f, 1.f);

    Color new_color = Color::Hsl(
        color_picker.value.hue(),
        color_picker.value.saturation(),
        lightness);
    color_picker.value = new_color;

    if (color_picker.always) {
      color_picker.callback(new_color);
    } else {
      color_picker.modified = true;
    }
  }
}

void ColorPickerSystem::focus_tree_leave(ElementPtr element) {
  auto& color_picker = element.color_picker();
  color_picker.open = false;
}

} // namespace datagui
