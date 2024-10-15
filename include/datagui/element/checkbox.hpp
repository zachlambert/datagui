#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"

namespace datagui {

struct Checkbox {
    bool checked;

    Checkbox(bool default_checked = false):
        checked(default_checked)
    {}
};

} // namespace datagui
