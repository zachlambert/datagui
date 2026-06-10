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

void GuiReader::number(dpack::NumberType type, void* value) {
  assert(node);
  if (in_color) {
    switch (type) {
    case dpack::NumberType::I32:
      *(std::int32_t*)value = color.data[color_i - 1] * 255;
      break;
    case dpack::NumberType::I64:
      *(std::int64_t*)value = color.data[color_i - 1] * 255;
      break;
    case dpack::NumberType::U32:
      *(std::int32_t*)value = color.data[color_i - 1] * 255;
      break;
    case dpack::NumberType::U64:
      *(std::uint64_t*)value = color.data[color_i - 1] * 255;
      break;
    case dpack::NumberType::F32:
      *(float*)value = color.data[color_i - 1];
      break;
    case dpack::NumberType::F64:
      *(double*)value = color.data[color_i - 1];
      break;
    case dpack::NumberType::U8:
      *(std::uint8_t*)value = color.data[color_i - 1] * 255;
      break;
    }
    return;
  }
#if 0
  if (auto constraint = get_constraint<dpack::ConstraintNumber>()) {
    if (auto range =
            std::get_if<dpack::ConstraintNumberRange>(&(*constraint))) {
      assert(node.type() == Type::Slider);
      double input = node.slider().value;
      switch (type) {
      case dpack::NumberType::I32:
        *(std::int32_t*)value = input;
        break;
      case dpack::NumberType::I64:
        *(std::int64_t*)value = input;
        break;
      case dpack::NumberType::U32:
        *(std::uint32_t*)value = input;
        break;
      case dpack::NumberType::U64:
        *(std::uint64_t*)value = input;
        break;
      case dpack::NumberType::U8:
        *(std::uint8_t*)value = input;
        break;
      case dpack::NumberType::F32:
        *(float*)value = input;
        break;
      case dpack::NumberType::F64:
        *(double*)value = input;
        break;
      }
      return;
    }
  }
#endif

  assert(node.type() == Type::TextInput);

  const auto& text = node.text_input().text;

  switch (type) {
  case dpack::NumberType::I32:
    *(std::int32_t*)value = number_from_string<std::int32_t>(text);
    break;
  case dpack::NumberType::I64:
    *(std::int64_t*)value = number_from_string<std::int64_t>(text);
    break;
  case dpack::NumberType::U32:
    *(std::uint32_t*)value = number_from_string<std::uint32_t>(text);
    break;
  case dpack::NumberType::U64:
    *(std::uint64_t*)value = number_from_string<std::uint64_t>(text);
    break;
  case dpack::NumberType::U8:
    *(std::uint8_t*)value = number_from_string<std::uint8_t>(text);
    break;
  case dpack::NumberType::F32:
    *(float*)value = number_from_string<float>(text);
    break;
  case dpack::NumberType::F64:
    *(double*)value = number_from_string<double>(text);
    break;
  }
}

bool GuiReader::boolean() {
  assert(node && node.type() == Type::Checkbox);
  return node.checkbox().checked;
}

const char* GuiReader::string() {
  assert(node && node.type() == Type::TextInput);
  return node.text_input().text.c_str();
}

int GuiReader::enumerate(const std::span<const char*>& labels) {
  assert(node && node.type() == Type::Select);
  return node.select().choice;
}

std::span<const std::uint8_t> GuiReader::binary() {
  assert(node && node.type() == Type::TextInput);
  try {
    binary_temp = dpack::base64_decode(node.text_input().text);
  } catch (const dpack::Base64Exception&) {
    // TODO: Handle text input constraints internally
    binary_temp.clear();
  }
  return binary_temp;
}

bool GuiReader::optional_begin() {
  node = node.child();
  assert(node && node.type() == Type::Checkbox);

  bool has_value = node.checkbox().checked;
  if (!has_value) {
    node = node.parent();
    return false;
  }
  node = node.next();
  assert(node);
  return true;
}

void GuiReader::optional_end() {
  node = node.parent();
  assert(node);
}

int GuiReader::variant_begin(const std::span<const char*>& labels) {
  node = node.child();
  assert(node && node.type() == Type::Select);
  int choice = node.select().choice;
  node = node.next();
  assert(node);
  return choice;
}

void GuiReader::variant_end() {
  node = node.parent();
  assert(node);
}

void GuiReader::object_begin() {
#if 0
  if (auto constraint = get_constraint<dpack::ConstraintObject>()) {
    if (std::get_if<dpack::ConstraintObjectColor>(&(*constraint))) {
      assert(node && node.type() == Type::ColorPicker);
      in_color = true;
      color = node.color_picker().value;
      color_i = 0;
      return;
    }
  }
#endif
  assert(node.type() == Type::Collapsable);
  node = node.child();
  assert(node);
  at_object_begin = true;
}

void GuiReader::object_next(const char* key) {
  if (in_color) {
    assert(color_i < 4);
    color_i++;
    return;
  }
  if (!at_object_begin) {
    node = node.next();
  }
  at_object_begin = false;
  assert(node);

  // The only time TextBox is used is for object labels so safe to check here
  // For compound objects, there is no text box label, instead it's a
  // collapsable with the label
  if (node.type() == Type::TextBox) {
    node = node.next();
    assert(node);
  } else {
    assert(node.type() == Type::Collapsable);
  }
}

void GuiReader::object_end() {
  if (in_color) {
    assert(color_i == 4);
    in_color = false;
    return;
  }
  if (!at_object_begin) {
    assert(node);
    assert(!node.next());
  }
  node = node.parent();
  assert(node);
  at_object_begin = false;
  in_color = false;
}

void GuiReader::tuple_begin() {
  assert(node && node.type() == Type::Collapsable);
  node = node.child();
  at_object_begin = true;
}

void GuiReader::tuple_next() {
  if (!at_object_begin) {
    node = node.next();
  }
  at_object_begin = false;
  assert(node);
}

void GuiReader::tuple_end() {
  if (at_object_begin) {
    if (node) {
      assert(false);
    }
  } else {
    if (node.next()) {
      assert(false);
    }
  }
  at_object_begin = false;
  node = node.parent();
  assert(node);
}

size_t GuiReader::list_begin() {
  // List =
  // collapsable      # List wrapper
  //   group          # Items wrapper
  //     group        # Item 0 wrapper
  //       <content>  # Item 0 content
  //       button     # Remove button
  //     group        # Item 1 wrapper
  //       <content>  # Item 1 content
  //       button     # Remove button
  //     etc...
  //   button         # New button

  // list wrapper -> items wrapper -> item 0 wrapper
  node = node.child().child();
  at_object_begin = true;

  return node.parent().size();
}

void GuiReader::list_next() {
  if (!at_object_begin) {
    // item (i-1) content -> item i wrapper
    node = node.parent().next();
  }
  at_object_begin = false;
  if (!node) {
    throw std::runtime_error("Incorrect number of tree nodes");
  }
  assert(node.type() == Type::Group);

  // item i wrapper -> item i content
  node = node.child();
  assert(node);
}

void GuiReader::list_end() {
  // Item n wrapper -> items wrapper -> list wrapper
  node = node.parent().parent();
}

} // namespace datagui
