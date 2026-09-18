#pragma once

#include "datagui/element/key_list.hpp"
#include "datagui/input/number_input.hpp"
#include <array>
#include <charconv>
#include <datapack/datapack.hpp>
#include <string>

namespace dgui {

NumberType convert_type(dpack::NumberType type);

template <typename T>
std::string number_to_string(T value) {
  std::array<char, 64> buffer;
  auto result =
      std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
  if (result.ec != std::errc{}) {
    return "0";
  }
  return std::string(buffer.data(), result.ptr);
}

struct ListVar {
  KeyList ids;
  bool dirty = false;
};

} // namespace dgui
