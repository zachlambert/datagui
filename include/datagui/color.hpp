#pragma once

#include <algorithm>
#include <array>
#include <cmath>

namespace datagui {

struct Color {
public:
  union {
    struct {
      float r;
      float g;
      float b;
      float a;
    };
    float data[4];
  };

  Color() {}
  Color(float r, float g, float b, float a = 1) : r(r), g(g), b(b), a(a) {}

  static Color Red(float value = 1) {
    return Color(value, 0, 0, 1);
  }
  static Color Green(float value = 1) {
    return Color(0, value, 0, 1);
  }
  static Color Blue(float value = 1) {
    return Color(0, 0, value, 1);
  }
  static Color Black() {
    return Color(0, 0, 0, 1);
  }
  static Color White() {
    return Color(1, 1, 1, 1);
  }
  static Color Gray(float value) {
    return Color(value, value, value, 1);
  }
  static Color Clear() {
    return Color(0, 0, 0, 0);
  }

  static Color Hsl(
      float hue,
      float saturation,
      float lightness,
      float alpha = 1) {
    // https://www.rapidtables.com/convert/color/hsl-to-rgb.html
    float c = (1 - std::fabs(2 * lightness - 1)) * saturation;
    float x = c * (1 - std::fabs(std::fmod(hue / 60, 2) - 1));
    float m = lightness - c / 2;

    std::array<float, 3> rgb;
    if (hue < 60) {
      rgb = {c, x, 0};
    } else if (hue < 120) {
      rgb = {x, c, 0};
    } else if (hue < 180) {
      rgb = {0, c, x};
    } else if (hue < 240) {
      rgb = {0, x, c};
    } else if (hue < 300) {
      rgb = {x, 0, c};
    } else {
      rgb = {c, 0, x};
    }

    Color color;
    color.r = rgb[0] + m;
    color.g = rgb[1] + m;
    color.b = rgb[2] + m;
    color.a = alpha;

    return color;
  }

  static Color Random() {
    Color color;
    color.r = float(rand()) / RAND_MAX;
    color.g = float(rand()) / RAND_MAX;
    color.b = float(rand()) / RAND_MAX;
    color.a = 1;
    return color;
  }

  bool equals(const Color& other, float max_error = 1e-6) const {
    float error = 1;
    error = std::max(error, std::abs(r - other.r));
    error = std::max(error, std::abs(g - other.g));
    error = std::max(error, std::abs(b - other.b));
    error = std::max(error, std::abs(a - other.a));
    return error <= max_error;
  }

  Color multiply(float factor) const {
    return Color(r * factor, g * factor, b * factor, a);
  }
};

} // namespace datagui
