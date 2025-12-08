#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <datapack/datapack.hpp>

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
      float alpha = 1);

  float hue() const;
  float saturation() const;
  float lightness() const;

  static Color Random();
  bool equals(const Color& other, float max_error = 1e-6) const;
  Color multiply(float factor) const;
};

} // namespace datagui

namespace datapack {

DATAPACK(datagui::Color);

} // namespace datapack
