#include "datagui/types/datapack.hpp"

namespace datagui {

void GuiWriter::number(datapack::NumberType type, const void* value) {
  std::string value_str;
  switch (type) {
  case datapack::NumberType::I32:
    value_str = std::to_string(*(std::int32_t*)value);
    break;
  case datapack::NumberType::I64:
    value_str = std::to_string(*(std::int64_t*)value);
    break;
  case datapack::NumberType::U32:
    value_str = std::to_string(*(std::uint32_t*)value);
    break;
  case datapack::NumberType::U64:
    value_str = std::to_string(*(std::uint64_t*)value);
    break;
  case datapack::NumberType::U8:
    value_str = std::to_string(*(std::uint8_t*)value);
    break;
  case datapack::NumberType::F32:
    value_str = std::to_string(*(float*)value);
    break;
  case datapack::NumberType::F64:
    value_str = std::to_string(*(double*)value);
    break;
  }

  gui.text_input_write(value_str);
}

void GuiWriter::boolean(bool value) {
  gui.checkbox_write(value);
}

void GuiWriter::string(const char* value) {
  std::string value_str(value);
  gui.text_input_write(value_str);
}

void GuiWriter::enumerate(int value, const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }
  gui.dropdown_write(labels_str, value);
}

void GuiWriter::binary(const std::span<const std::uint8_t>& data) {
  gui.text_input("<binary not editable>");
}

void GuiWriter::optional_begin(bool has_value) {
  gui.series_begin_force();
  gui.variable<bool>(has_value);
  gui.checkbox_write(has_value);
  if (has_value) {
    gui.series_begin_force();
  }
}

void GuiWriter::optional_end() {
  gui.series_end();
}

void GuiWriter::variant_begin(int value, const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }
  gui.dropdown_write(labels_str, value);
  gui.series_begin_force();
}
void GuiWriter::variant_end() {
  gui.series_end();
}

void GuiWriter::object_begin() {
  gui.series_begin_force();
}
void GuiWriter::object_next(const char* key) {
  gui.text_box(key);
}
void GuiWriter::object_end() {
  gui.series_end();
}

void GuiWriter::tuple_begin() {
  gui.series_begin_force();
}
void GuiWriter::tuple_next() {
  // Do nothing
}
void GuiWriter::tuple_end() {
  gui.series_end();
}

void GuiWriter::list_begin() {
  gui.series_begin_force();
  list_sizes.push(gui.variable<std::size_t>(0));
}
void GuiWriter::list_next() {
  list_sizes.top().mut()++;
}
void GuiWriter::list_end() {
  gui.button("Push");
  gui.button("Pop");
  gui.series_end();
}

void GuiReader::number(datapack::NumberType type, void* value) {
  auto value_str = gui.text_input_read();
  try {
    switch (type) {
    case datapack::NumberType::I32:
      *(std::int32_t*)value = std::stoi(value_str);
      break;
    case datapack::NumberType::I64:
      *(std::int64_t*)value = std::stol(value_str);
      break;
    case datapack::NumberType::U32:
      *(std::uint32_t*)value = std::stoul(value_str);
      break;
    case datapack::NumberType::U64:
      *(std::uint64_t*)value = std::stoul(value_str);
      break;
    case datapack::NumberType::U8:
      *(std::uint8_t*)value = std::stoul(value_str);
      break;
    case datapack::NumberType::F32:
      *(float*)value = std::stof(value_str);
      break;
    case datapack::NumberType::F64:
      *(double*)value = std::stod(value_str);
      break;
    }
  } catch (const std::runtime_error& e) {
    // Ignore
  }
}

bool GuiReader::boolean() {
  return gui.checkbox_read();
}

const char* GuiReader::string() {
  return gui.text_input_read().c_str();
}

int GuiReader::enumerate(const std::span<const char*>& labels) {
  return gui.dropdown_read();
}

std::span<const std::uint8_t> GuiReader::binary() {
  gui.text_box("Binary not implemented");
  return std::span<const std::uint8_t>((const std::uint8_t*)nullptr, 0);
}

bool GuiReader::optional_begin() {
  gui.series_begin_force();
  auto has_value = gui.variable<bool>();
  gui.checkbox(has_value);
  if (*has_value) {
    gui.series_begin_force();
    return true;
  }
  return false;
}

void GuiReader::optional_end() {
  gui.series_end();
}

int GuiReader::variant_begin(const std::span<const char*>& labels) {
  int value = gui.dropdown_read();
  gui.series_begin_force();
  return value;
}
void GuiReader::variant_end() {
  gui.series_end();
}

void GuiReader::object_begin() {
  gui.series_begin();
}
void GuiReader::object_next(const char* key) {
  gui.text_box(key);
}
void GuiReader::object_end() {
  gui.series_end();
}

void GuiReader::tuple_begin() {
  gui.series_begin();
}
void GuiReader::tuple_next() {
  // Do nothing
}
void GuiReader::tuple_end() {
  gui.series_end();
}

void GuiReader::list_begin() {
  gui.series_begin();
  auto size = gui.variable<std::size_t>(0);
  list_sizes.push(std::make_pair(size, *size));
}
bool GuiReader::list_next() {
  auto& [variable, remaining] = list_sizes.top();
  if (remaining == 0) {
    return false;
  }
  remaining--;
  return true;
}
void GuiReader::list_end() {
  auto size = list_sizes.top().first;
  if (gui.button("Push")) {
    size.set(*size + 1);
  }
  if (gui.button("Pop") && *size > 0) {
    size.set(*size - 1);
  }
  list_sizes.pop();
  gui.series_end();
}

} // namespace datagui
