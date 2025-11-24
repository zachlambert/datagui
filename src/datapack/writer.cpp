#include "datagui/datapack/writer.hpp"
#include "datagui/gui.hpp"
#include <datapack/encode/base64.hpp>

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

  gui.text_input(value_str, {});
}

void GuiWriter::boolean(bool value) {
  gui.checkbox(value, {});
}

void GuiWriter::string(const char* value) {
  gui.text_input(std::string(value), {});
}

void GuiWriter::enumerate(int value, const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }
  gui.dropdown(labels_str, value, {});
}

void GuiWriter::binary(const std::span<const std::uint8_t>& data) {
  std::string text = datapack::base64_encode(data);
  gui.text_input(text, {});
}

void GuiWriter::optional_begin(bool has_value) {
  gui.args().tight();
  gui.series();
  auto has_value_var = gui.variable<bool>(has_value);
  gui.checkbox(has_value_var);
  gui.depend_variable(has_value_var);
  if (!has_value) {
    gui.end();
  }
}

void GuiWriter::optional_end() {
  gui.end();
}

void GuiWriter::variant_begin(int value, const std::span<const char*>& labels) {
  std::vector<std::string> labels_str;
  for (auto label : labels) {
    labels_str.emplace_back(label);
  }

  gui.args().tight();
  gui.series();
  auto choice_var = gui.variable<int>(value);
  gui.dropdown(labels_str, choice_var);
  gui.depend_variable(choice_var);
  gui.key(*choice_var);
}

void GuiWriter::variant_end() {
  gui.end();
}

void GuiWriter::object_begin() {
  gui.args().tight();
  gui.series();
  at_object_begin = true;
}

void GuiWriter::object_next(const char* key) {
  if (!at_object_begin) {
    gui.end();
  }
  at_object_begin = false;
  gui.section(key);
}

void GuiWriter::object_end() {
  if (!at_object_begin) {
    gui.end();
  }
  at_object_begin = false;
  gui.end();
}

void GuiWriter::tuple_begin() {
  gui.args().tight();
  gui.series();
}

void GuiWriter::tuple_next() {
  // Do nothing
}

void GuiWriter::tuple_end() {
  gui.end();
}

void GuiWriter::list_begin() {
  gui.args().tight();
  gui.series();

  auto key_list = gui.variable<KeyList>();
  ListState state = {key_list, 0};
  list_stack.push(state);

  gui.args().tight();
  gui.series();
  gui.depend_variable(key_list);
}

void GuiWriter::list_next() {
  assert(!list_stack.empty());
  auto& state = list_stack.top();

  if (state.index != 0) {
    auto keys = state.keys;
    std::size_t remove_i = state.index - 1;
    gui.button("Remove", [=]() { keys.mut().remove(remove_i); });
    gui.end();
  }

  assert(state.index == state.keys->size());
  std::size_t new_key = state.keys.mut().append();

  gui.key((*state.keys)[state.index]);
  gui.args().horizontal();
  gui.series();

  state.index++;
}

void GuiWriter::list_end() {
  assert(!list_stack.empty());
  const auto& state = list_stack.top();
  auto keys = state.keys;

  if (state.index != 0) {
    std::size_t remove_i = state.index - 1;
    gui.button("Remove", [=]() { keys.mut().remove(remove_i); });
    gui.end();
  }

  gui.end();

  gui.button("new", [=]() { keys.mut().append(); });
  gui.end();

  list_stack.pop();
}

} // namespace datagui
