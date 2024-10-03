#pragma once

namespace datagui {

struct Color {
    float r;
    float g;
    float b;

    Color() {}
    Color(float r, float g, float b):
        r(r), g(g), b(b)
    {}

    static Color Red(float value=1) {
        return Color(value, 0, 0);
    }
    static Color Green(float value=1) {
        return Color(0, value, 0);
    }
    static Color Blue(float value=1) {
        return Color(0, 0, value);
    }
    static Color Black() {
        return Color(0, 0, 0);
    }
    static Color White() {
        return Color(1, 1, 1);
    }
    static Color Gray(float value) {
        return Color(value, value, value);
    }
};

} // namespace datagui
