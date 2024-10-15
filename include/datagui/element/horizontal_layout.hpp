#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"

namespace datagui {

struct HorizontalLayout {
    Vecf input_size;

    HorizontalLayout(const Vecf& input_size):
        input_size(input_size)
    {}
};

} // namespace datagui
