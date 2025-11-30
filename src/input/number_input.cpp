#include "datagui/input/number_input.hpp"
#include <charconv>

namespace datagui {

template <typename T>
bool text_to_number(const std::string& text, T& value) {
  auto result = std::from_chars(text.data(), text.data() + text.size(), value);
  return (result.ptr == text.data() + text.size()) && result.ec == std::errc{};
}

#define INSTANTIATE(T) template bool text_to_number<T>(const std::string&, T&);
INSTANTIATE(std::int32_t)
INSTANTIATE(std::int64_t)
INSTANTIATE(std::uint32_t)
INSTANTIATE(std::uint64_t)
INSTANTIATE(float)
INSTANTIATE(double)
INSTANTIATE(std::uint8_t)
#undef INSTANTIATE

template <typename T>
bool valid_text_to_number(const std::string& text) {
  T value;
  auto result = std::from_chars(text.data(), text.data() + text.size(), value);
  bool valid =
      (result.ptr == text.data() + text.size()) && result.ec == std::errc{};
  return valid;
}

bool valid_text_to_number(NumberType type, const std::string& text) {
  switch (type) {
  case NumberType::I32:
    return valid_text_to_number<std::int32_t>(text);
  case NumberType::I64:
    return valid_text_to_number<std::int64_t>(text);
  case NumberType::U32:
    return valid_text_to_number<std::uint32_t>(text);
  case NumberType::U64:
    return valid_text_to_number<std::uint64_t>(text);
  case NumberType::F32:
    return valid_text_to_number<float>(text);
  case NumberType::F64:
    return valid_text_to_number<double>(text);
  case NumberType::U8:
    return valid_text_to_number<std::uint8_t>(text);
  }
  return false;
}

} // namespace datagui
