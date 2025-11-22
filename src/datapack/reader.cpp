#include "datagui/datapack/reader.hpp"
#include <charconv>
#include <datapack/encode/base64.hpp>

namespace datagui {

template <typename T>
T number_from_string(const std::string& string) {
  T value;
  auto error =
      std::from_chars(string.data(), string.data() + string.size(), value).ec;
  if (error != std::errc{}) {
    return T(0);
  }
  return value;
}

void GuiReader::number(datapack::NumberType type, void* value) {
  if (!node || node.type() != Type::TextInput) {
    invalidate();
    return;
  }
  const auto& text = node.text_input().text;

  switch (type) {
  case datapack::NumberType::I32:
    *(std::int32_t*)value = number_from_string<std::int32_t>(text);
    break;
  case datapack::NumberType::I64:
    *(std::int64_t*)value = number_from_string<std::int64_t>(text);
    break;
  case datapack::NumberType::U32:
    *(std::uint32_t*)value = number_from_string<std::uint32_t>(text);
    break;
  case datapack::NumberType::U64:
    *(std::uint64_t*)value = number_from_string<std::uint64_t>(text);
    break;
  case datapack::NumberType::U8:
    *(std::uint8_t*)value = number_from_string<std::uint8_t>(text);
    break;
  case datapack::NumberType::F32:
    *(float*)value = number_from_string<float>(text);
    break;
  case datapack::NumberType::F64:
    *(double*)value = number_from_string<double>(text);
    break;
  }
}

bool GuiReader::boolean() {
  if (!node || node.type() != Type::Checkbox) {
    invalidate();
    return false;
  }
  return node.checkbox().checked;
}

const char* GuiReader::string() {
  if (!node || node.type() != Type::TextInput) {
    invalidate();
    return "";
  }
  return node.text_input().text.c_str();
}

int GuiReader::enumerate(const std::span<const char*>& labels) {
  if (!node || node.type() != Type::Dropdown) {
    invalidate();
    return 0;
  }
  return node.dropdown().choice;
}

std::span<const std::uint8_t> GuiReader::binary() {
  if (!node || node.type() != Type::TextInput) {
    invalidate();
    return {};
  }
  try {
    binary_temp = datapack::base64_decode(node.text_input().text);
  } catch (const datapack::Base64Exception&) {
    // TODO: Handle text input constraints internally
    binary_temp.clear();
  }
  return binary_temp;
}

bool GuiReader::optional_begin() {
  node = node.child();

  if (!node || node.type() != Type::Checkbox) {
    invalidate();
    node = node.parent();
    return false;
  }
  bool has_value = node.checkbox().checked;
  if (!has_value) {
    node = node.parent();
    return false;
  }
  node = node.next();
  return true;
}

void GuiReader::optional_end() {
  node = node.parent();
}

int GuiReader::variant_begin(const std::span<const char*>& labels) {
  node = node.child();

  if (!node || node.type() != Type::Dropdown) {
    invalidate();
    node = node.parent();
    return 0;
  }
  int choice = node.dropdown().choice;
  node = node.next();
  return choice;
}

void GuiReader::variant_end() {
  node = node.parent();
}

void GuiReader::object_begin() {
  if (!node || node.type() != Type::Series) {
    invalidate();
    return;
  }
  node = node.child();
  at_object_begin = true;
}

void GuiReader::object_next(const char* key) {
  if (!at_object_begin) {
    node = node.parent().next();
  }
  at_object_begin = false;
  node = node.child();
}

void GuiReader::object_end() {
  if (!at_object_begin) {
    if (node.next()) {
      invalidate();
    }
    node = node.parent();
  } else {
    if (node.child()) {
      invalidate();
    }
  }
  at_object_begin = false;
  node = node.parent();
}

void GuiReader::tuple_begin() {
  if (!node || node.type() != Type::Series) {
    invalidate();
    return;
  }
  node = node.child();
  at_object_begin = true;
}

void GuiReader::tuple_next() {
  if (!at_object_begin) {
    node = node.next();
  }
  at_object_begin = false;
}

void GuiReader::tuple_end() {
  if (at_object_begin) {
    if (node) {
      invalidate();
    }
  } else {
    if (node.next()) {
      invalidate();
    }
  }
  at_object_begin = false;
  node = node.parent();
}

void GuiReader::list_begin() {
  // Skip over button section
  node = node.child().next().child();
  at_object_begin = true;
}

bool GuiReader::list_next() {
  if (at_object_begin) {
    at_object_begin = false;
    return node;
  }
  node = node.next();
  return node;
}

void GuiReader::list_end() {
  at_object_begin = false;
  assert(at_object_begin && !node || !node.next());
  node = node.parent().parent();
}

} // namespace datagui
