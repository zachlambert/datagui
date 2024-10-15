#pragma once

#include "datagui/color.hpp"

namespace datagui {

enum class Font {
    DejaVuSans,
    DejaVuSerif,
    DejaVuSansMono
};

struct Style {
    struct Text {
        Font font = Font::DejaVuSans;
        float font_size = 16;
        float padding = 4;
        float line_height = 1;
        Color font_color = Color::Black();
    };

    struct Element {
        Color bg_color = Color::White();
        float padding = 4;
        float border_width = 4;
        Color border_color = Color::Black();
        Color focus_color = Color(0, 1, 1);
        Color hovered_bg_color = Color::Gray(0.75);
        Color pressed_bg_color = Color::Gray(0.5);
    };

    struct TextInput {
        int cursor_width = 2;
        float cursor_blink_period = 0.5;
        Color cursor_color = Color::Gray(0.25);
        Color highlight_color = Color::Gray(0.75);
    };

    struct Checkbox {
        float size = 1;
        float check_padding = 2;
        Color check_color = Color(0, 1, 1);
    };

    Color bg_color = Color::Gray(0.8);
    Text text;
    Element element;
    TextInput text_input;
    Checkbox checkbox;
};

} // namespace datagui
