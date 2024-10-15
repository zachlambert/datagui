#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"

namespace datagui {

struct VerticalLayout {
    Vecf input_size;

    VerticalLayout(
        const Vecf& input_size
    ):
        input_size(input_size)
    {}
};

} // namespace datagui
