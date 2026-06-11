#pragma once

#include "datagui/element/key_list.hpp"
#include "datagui/input/number_input.hpp"
#include <datapack/datapack.hpp>

namespace datagui {

NumberType convert_type(dpack::NumberType type);

struct ListVar {
  KeyList ids;
  bool dirty = false;
};

} // namespace datagui
