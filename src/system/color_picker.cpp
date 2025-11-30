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

  float text_height = fm->text_height(theme->text_font, theme->text_size);
  state.fixed_size = Vec2::uniform(text_height);
  state.dynamic_size = Vec2();

  Vec2 float_size = Vec2(
      theme->color_picker_hue_wheel_radius * 2 +
          2 * theme->color_picker_value_scale_width,
      theme->color_picker_hue_wheel_radius * 2);

  state.floating = color_picker.open;
  state.floating_type = FloatingTypeRelative(Vec2(), float_size);
}

void ColorPickerSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& color_picker = element.color_picker();

  renderer.queue_box(state.box(), Color::Gray(0.5));

  float value = color_picker.value.value();
  struct Pixel {
    std::uint8_t r, g, b, a;
  };
  {
    const std::size_t n = 100;
    std::vector<Pixel> pixels(n * n);
    for (std::size_t i = 0; i < n; i++) {
      for (std::size_t j = 0; j < n; j++) {
        Pixel& pixel = pixels[i * n + j];
        float x = (float(j) - float(n) / 2) / n;
        float y = (float(i) - float(n) / 2) / n;
        float saturation = std::hypot(x, y);
        if (saturation > 1) {
          pixel.a = 0;
          continue;
        }
        pixel.a = 255;
        float hue = 360 * std::atan2(-y, -x);
        Color color = Color::Hsl(hue, saturation, value);
        pixel.r = color.r * 255;
        pixel.g = color.g * 255;
        pixel.b = color.b * 255;
      }
    }
    Vec2 pos = state.position;
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
        pixel.g = value;
        pixel.a = 255;
      }
    }
    Vec2 pos = state.position;
    pos.x += theme->color_picker_hue_wheel_radius * 2 +
             theme->color_picker_value_scale_width;
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
  return false;
}

} // namespace datagui
