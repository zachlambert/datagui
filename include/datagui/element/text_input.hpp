#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include <string>

namespace datagui {

struct TextInput {
    float max_width;
    std::string text;
    bool changed;

    TextInput(
        const std::string& default_text,
        float max_width
    ):
        max_width(max_width),
        text(default_text),
        changed(false)
    {}
};

} // namespace datagui
