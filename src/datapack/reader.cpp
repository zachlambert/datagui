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
  if (!node) {
    invalidate();
    return;
  }
  if (in_color) {
    switch (type) {
    case datapack::NumberType::I32:
      *(std::int32_t*)value = color.data[color_i - 1] * 255;
      break;
    case datapack::NumberType::I64:
      *(std::int64_t*)value = color.data[color_i - 1] * 255;
      break;
    case datapack::NumberType::U32:
      *(std::int32_t*)value = color.data[color_i - 1] * 255;
      break;
    case datapack::NumberType::U64:
      *(std::uint64_t*)value = color.data[color_i - 1] * 255;
      break;
    case datapack::NumberType::F32:
      *(float*)value = color.data[color_i - 1];
      break;
    case datapack::NumberType::F64:
      *(double*)value = color.data[color_i - 1];
      break;
    case datapack::NumberType::U8:
      *(std::uint8_t*)value = color.data[color_i - 1] * 255;
      break;
    }
    return;
  }
  if (auto constraint = get_constraint<datapack::ConstraintNumber>()) {
    if (auto range =
            std::get_if<datapack::ConstraintNumberRange>(&(*constraint))) {
      if (node.type() != Type::Slider) {
        invalidate();
        return;
      }
      double input = node.slider().value;
      switch (type) {
      case datapack::NumberType::I32:
        *(std::int32_t*)value = input;
        break;
      case datapack::NumberType::I64:
        *(std::int64_t*)value = input;
        break;
      case datapack::NumberType::U32:
        *(std::uint32_t*)value = input;
        break;
      case datapack::NumberType::U64:
        *(std::uint64_t*)value = input;
        break;
      case datapack::NumberType::U8:
        *(std::uint8_t*)value = input;
        break;
      case datapack::NumberType::F32:
        *(float*)value = input;
        break;
      case datapack::NumberType::F64:
        *(double*)value = input;
        break;
      }
      return;
    }
  }

  if (node.type() != Type::TextInput) {
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
  assert(node);
  return choice;
}

void GuiReader::variant_end() {
  node = node.parent();
}

void GuiReader::object_begin() {
  if (auto constraint = get_constraint<datapack::ConstraintObject>()) {
    if (std::get_if<datapack::ConstraintObjectColor>(&(*constraint))) {
      if (!node || node.type() != Type::ColorPicker) {
        invalidate();
        return;
      }
      in_color = true;
      color = node.color_picker().value;
      color_i = 0;
      return;
    }
  }
  if (!node || node.type() != Type::Series) {
    invalidate();
    return;
  }
  node = node.child();
  at_object_begin = true;
}

void GuiReader::object_next(const char* key) {
  if (in_color) {
    if (color_i > 3) {
      invalidate();
      return;
    }
    color_i++;
    return;
  }
  if (!at_object_begin) {
    node = node.parent().next();
  }
  at_object_begin = false;
  node = node.child();
}

void GuiReader::object_end() {
  if (in_color) {
    if (color_i != 4) {
      invalidate();
    }
    in_color = false;
    node = node.next();
    return;
  }
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
  in_color = false;
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
  // List =
  // series           # List wrapper
  //   series         # Items wrapper
  //     series       # Item 0 wrapper
  //       <content>  # Item 0 content
  //       button     # Remove button
  //     series       # Item 1 wrapper
  //       <content>  # Item 1 content
  //       button     # Remove button
  //     etc...
  //   button         # New button

  // list wrapper -> items wrapper -> item 0 wrapper
  node = node.child().child();
  at_object_begin = true;
}

bool GuiReader::list_next() {
  if (!at_object_begin) {
    // item (i-1) content -> item i wrapper
    node = node.parent().next();
  }
  at_object_begin = false;
  if (!node) {
    return false;
  }
  // item i wrapper -> item i content
  node = node.child();
  assert(node);
  return true;
}

void GuiReader::list_end() {
  // Item n wrapper -> items wrapper -> list wrapper
  node = node.parent().parent();
}

} // namespace datagui
