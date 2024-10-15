#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include <string>

namespace datagui {

struct Button {
    std::string text;
    float max_width;

    Button(
        const std::string& text,
        float max_width
    ):
        text(text),
        max_width(max_width)
    {}
};

} // namespace datagui
