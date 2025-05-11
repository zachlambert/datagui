#include "datagui/types/reader.hpp"

namespace datagui {

void GuiReader::integer(datapack::IntType type, void* value) {
  gui.text_input();
}

void GuiReader::floating(datapack::FloatType type, void* value) {
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

std::tuple<const std::uint8_t*, std::size_t> GuiReader::binary(
    std::size_t length,
    std::size_t stride) {
  gui.text_box("Binary not implemented");
  return std::make_tuple<const std::uint8_t*, std::size_t>(nullptr, 0);
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

void GuiReader::object_begin(std::size_t) {
  gui.series_begin();
}
void GuiReader::object_end(std::size_t) {
  gui.series_end();
}
void GuiReader::object_next(const char* key) {
  gui.text_box(key);
}

void GuiReader::tuple_begin(std::size_t trivial_size) {
  gui.series_begin();
}
void GuiReader::tuple_end(std::size_t trivial_size) {
  gui.series_end();
}
void GuiReader::tuple_next() {
  // Do nothing
}

void GuiReader::list_begin(bool is_trivial) {
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
