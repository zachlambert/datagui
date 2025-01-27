#include "datagui/internal/reader.hpp"
#include "datagui/window.hpp"
#include <datapack/encode/base64.hpp>
#include <exception>

namespace datapack {

GuiReader::GuiReader(datagui::Window& window, bool& changed) : window(window), changed(changed) {
  changed = false;
  window.vertical_layout(0, 0, "", true);
}

GuiReader::~GuiReader() {
  //
  window.layout_end();
}

void GuiReader::integer(IntType type, void* value) {
  const std::string& output = *window.text_input("", 0, next_key, true);
  try {
    switch (type) {
    case IntType::I32:
      *((std::int32_t*)value) = std::stoi(output);
      break;
    case IntType::I64:
      *((std::int32_t*)value) = std::stoll(output);
      break;
    case IntType::U32:
      *((std::int32_t*)value) = std::stoul(output);
      break;
    case IntType::U64:
      *((std::int32_t*)value) = std::stoull(output);
      break;
    case IntType::U8:
      *((std::uint8_t*)value) = std::clamp(std::stoul(output), 0ul, 255ul);
      break;
    }
  } catch (std::invalid_argument&) {
    return;
  }

  changed = true;
}

void GuiReader::floating(FloatType type, void* value) {
  const std::string& output = *window.text_input("", -1, next_key, true);
  try {
    switch (type) {
    case FloatType::F32:
      *((float*)value) = std::stof(output);
      break;
    case FloatType::F64:
      *((double*)value) = std::stod(output);
      break;
    }
  } catch (std::invalid_argument&) {
    return;
  }

  changed = true;
}

bool GuiReader::boolean() { return *window.checkbox(next_key, true); }

const char* GuiReader::string() { return window.text_input("", -1, next_key, true)->c_str(); }

int GuiReader::enumerate(const std::span<const char*>& labels) {
  // TODO: Allow selection to take a span of const char*
  std::vector<std::string> choices;
  for (auto label : labels) {
    choices.push_back(std::string(label));
  }
  return *window.selection(choices, 0, -1, next_key, true);
}

std::tuple<const std::uint8_t*, std::size_t> GuiReader::binary(
    std::size_t length,
    std::size_t stride) {
  const std::string& output = *window.text_input("", -1, next_key, true);
  data_temp = base64_decode(output);
  return std::make_tuple(data_temp.data(), data_temp.size());
}

bool GuiReader::optional_begin() {
  if (window.checkbox(next_key)) {
    window.vertical_layout(0, 0, "", true);
    return true;
  }
  return false;
}

void GuiReader::optional_end() { window.layout_end(); }

int GuiReader::variant_begin(const std::span<const char*>& labels) {
  // TODO: Allow selection to take a span of const char*
  std::vector<std::string> choices;
  for (auto label : labels) {
    choices.push_back(std::string(label));
  }
  int choice = *window.selection(choices, 0, -1, next_key, true);
  window.vertical_layout(0, 0, "", true);
  return choice;
}

void GuiReader::variant_end() { window.layout_end(); }

void GuiReader::object_begin(std::size_t) {
  window.vertical_layout(-1, 0, "", true);
}

void GuiReader::object_end(std::size_t) {
  //
  window.layout_end();
}

void GuiReader::object_next(const char* key) {
  window.text(key, 0, key + std::string("_key"));
  next_key = key;
}

void GuiReader::tuple_begin(std::size_t trivial_size) { window.vertical_layout(); }

void GuiReader::tuple_end(std::size_t trivial_size) { window.layout_end(); }

void GuiReader::tuple_next() { next_key = ""; }

void GuiReader::list_begin(bool is_trivial) { window.vertical_layout(); }

bool GuiReader::list_next() {
  next_key = "";
  // TODO: Handle a way to add/remove
  return false;
}

void GuiReader::list_end() { window.layout_end(); }

} // namespace datapack
