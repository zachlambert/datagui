#pragma once

#include <string>
#include "datagui/color.hpp"
#include "datagui/geometry.hpp"

namespace datagui {

struct Text {
    std::string text;
    float max_width;

    Text(
        const std::string& text,
        float max_width
    ):
        text(text),
        max_width(max_width)
    {}
};

} // namespace datagui
