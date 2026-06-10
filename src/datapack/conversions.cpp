#include "datagui/datapack/conversions.hpp"

namespace datagui {

NumberType convert_type(dpack::NumberType type) {
  switch (type) {
  case dpack::NumberType::I32:
    return NumberType::I32;
  case dpack::NumberType::I64:
    return NumberType::I64;
  case dpack::NumberType::U32:
    return NumberType::U32;
  case dpack::NumberType::U64:
    return NumberType::U64;
  case dpack::NumberType::F32:
    return NumberType::F32;
  case dpack::NumberType::F64:
    return NumberType::F64;
  case dpack::NumberType::U8:
    return NumberType::U8;
  }
  return NumberType::I32; // Unreachable
}

} // namespace datagui
