#pragma once

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
};

} // namespace datagui
