#pragma once

#include "datagui/color.hpp"
#include <cstddef>

namespace dgui {

size_t ansi_sequence_match(
    const char* text,
    size_t length,
    const Color& default_color,
    Color& color);

} // namespace dgui
