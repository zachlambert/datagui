#pragma once

#include <datapack/schema/schema.hpp>

namespace datagui {

class Gui;
void datapack_edit(
    Gui& gui,
    const std::string& next_label,
    const datapack::Schema& schema);

} // namespace datagui
