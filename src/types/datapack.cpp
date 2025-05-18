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

void GuiWriter::string(const char*) {}
void GuiWriter::enumerate(int value, const char* label) {}
void GuiWriter::binary(const std::span<const std::uint8_t>& data) {}

void GuiWriter::optional_begin(bool has_value) {}
void GuiWriter::optional_end() {}

void GuiWriter::variant_begin(int value, const char* label) {}
void GuiWriter::variant_end() {}

void GuiWriter::object_begin() {}
void GuiWriter::object_next(const char* key) {}
void GuiWriter::object_end() {}

void GuiWriter::tuple_begin() {}
void GuiWriter::tuple_next() {}
void GuiWriter::tuple_end() {}

void GuiWriter::list_begin() {}
void GuiWriter::list_next() {}
void GuiWriter::list_end() {}

void GuiReader::number(datapack::NumberType type, void* value) {
  gui.text_input();
}

bool GuiReader::boolean() {
  gui.checkbox();
  return false;
}

const char* GuiReader::string() {
  gui.text_input();
  return "";
}

int GuiReader::enumerate(const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.push_back(std::string(label));
  }
  gui.dropdown(labels_str);
  return 0;
}

std::span<const std::uint8_t> GuiReader::binary() {
  gui.text_box("Binary not implemented");
  return std::span<const std::uint8_t>((const std::uint8_t*)nullptr, 0);
}

bool GuiReader::optional_begin() {
  auto checked = gui.variable<bool>(false);
  gui.checkbox(checked);
  if (*checked) {
    gui.series_begin();
    return true;
  }
  return false;
}
void GuiReader::optional_end() {
  gui.series_end();
}

int GuiReader::variant_begin(const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.push_back(std::string(label));
  }
  gui.dropdown(labels_str);
  gui.series_begin();
  return 0;
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
}
bool GuiReader::list_next() {
  // TODO
  return false;
}
void GuiReader::list_end() {
  gui.series_end();
}

} // namespace datagui
