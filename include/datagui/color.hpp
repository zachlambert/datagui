#pragma once

#include <algorithm>

namespace datagui {

struct Color {
    float r;
    float g;
    float b;
    float a;

    Color() {}
    Color(float r, float g, float b, float a = 1):
        r(r), g(g), b(b), a(a)
    {}

    static Color Red(float value=1) {
        return Color(value, 0, 0, 1);
    }
    static Color Green(float value=1) {
        return Color(0, value, 0, 1);
    }
    static Color Blue(float value=1) {
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

    bool equals(const Color& other, float max_error = 1e-6) const {
        float error = 1;
        error = std::max(error, std::abs(r - other.r));
        error = std::max(error, std::abs(g - other.g));
        error = std::max(error, std::abs(b - other.b));
        error = std::max(error, std::abs(a - other.a));
        return error <= max_error;
    }
};

} // namespace datagui
