#pragma once

#include <cstdint>
#include <string>

namespace datagui {

enum class NumberType { I32, I64, U32, U64, F32, F64, U8 };

template <typename T>
struct number_type_details {};

template <typename T>
constexpr NumberType number_type() {
  return number_type_details<T>::value;
}

#define NUMBER_TYPE(type, Type) \
  template <> \
  struct number_type_details<type> { \
    static constexpr NumberType value = NumberType::Type; \
  };

NUMBER_TYPE(std::int32_t, I32);
NUMBER_TYPE(std::int64_t, I64);
NUMBER_TYPE(std::uint32_t, U32);
NUMBER_TYPE(std::uint64_t, U64);
NUMBER_TYPE(float, F32);
NUMBER_TYPE(double, F64);
NUMBER_TYPE(std::uint8_t, U8);

#undef EDIT_TYPE

template <typename T>
bool text_to_number(const std::string& text, T& value);

bool valid_text_to_number(NumberType type, const std::string& text);

} // namespace datagui
