#pragma once

#include <datapack/schema/schema.hpp>

namespace datagui {

class Gui;
bool datapack_edit(Gui& gui, const datapack::Schema& schema);

} // namespace datagui
